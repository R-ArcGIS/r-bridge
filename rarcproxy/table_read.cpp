#include "stdafx.h"
#include "table_read.h"
#include "tools.h"
#include "r_geometry.h"

template<typename T>
void fl<T>::move_to(SEXP df)
{
  tools::protect pt;
  SEXP pair = tools::newPair(tools::newVal(vect1), R_NilValue, pt);
  tools::nameIt(pair, (LPCSTR)name);
  tools::push_back(df, pair);
  vect1.clear();
}

template<typename T>
SEXP fl<T>::getSEXP() const
{
  return tools::newVal(vect1);
}

template<int ROWS>
inline int mxoffset( int i, int j) { return i + ROWS * j;}

template<>
void fl<bool>::push(CComVariant &v)
{
  if (v.vt == VT_EMPTY || v.vt == VT_NULL)
    v = false;
  else
    v.ChangeType(VT_BOOL);
  vect1.push_back(v.boolVal != VARIANT_FALSE ? true : false);
}

template<>
void fl<int>::push(CComVariant &v)
{
  if (v.vt == VT_EMPTY || v.vt == VT_NULL)
    v = -std::numeric_limits<long>::max();
  else
    v.ChangeType(VT_I4);
  vect1.push_back(v.lVal);
}

template<>
void fl<double>::push(CComVariant &v)
{
  if (v.vt == VT_EMPTY || v.vt == VT_NULL)
    v = std::numeric_limits<double>::quiet_NaN();
  else
    v.ChangeType(VT_R8);
  vect1.push_back(v.dblVal);
}

template<>
void fl<std::wstring>::push(CComVariant &v)
{
  if (v.vt == VT_EMPTY || v.vt == VT_NULL)
    vect1.push_back(L"");
  else
  {
    v.ChangeType(VT_BSTR);
    vect1.push_back(v.bstrVal);
  }
}

void extractSS(IDisplayTable* pDT, ISelectionSet **ppSS, BSTR *where_)
{
  if (!pDT)
    return;

  CComQIPtr<IDisplayTable> ipDTable(pDT);

  CComQIPtr<ISelectionSet> ipSelectionSet;
  if (pDT->get_DisplaySelectionSet(&ipSelectionSet) != S_OK)
  {
    CComQIPtr<ITableSelection> ipTableSelection(ipDTable);
    if (ipTableSelection) ipTableSelection->get_SelectionSet(&ipSelectionSet);
  }

  if (ipSelectionSet)
  {
    long n = 0;
    ipSelectionSet->get_Count(&n);
    if (n > 0) //real selection 
    {
      *ppSS = ipSelectionSet.Detach();
      return;
    }
/*
    CComQIPtr<ITableSelection2> ipTableSel2(pDT);
    if (ipTableSel2)
    {
      VARIANT_BOOL b = VARIANT_FALSE;
      ipTableSel2->get_HasSelection(&b);
      if (b != VARIANT_FALSE) // 0 selection
      {
        *ppSS = ipSelectionSet.Detach();
        return;
      }
    }
*/
    ipSelectionSet.Release();
  }

  //now check Definition
  CComQIPtr<ITableDefinition> ipDefinition(pDT);
  if (ipDefinition)
    ipDefinition->get_DefinitionSelectionSet(&ipSelectionSet);

  if (ipSelectionSet)
  {
    //this is layer created from selection set
    long n = 0;
    ipSelectionSet->get_Count(&n);
    if (n > 0)
      *ppSS = ipSelectionSet.Detach();
  }

  if (where_ && ipDefinition)
    ipDefinition->get_DefinitionExpression(where_);
}


HRESULT load_from_cursor(ICursor* pCursor, const std::vector<std::wstring> &fields, std::vector<fl_base*> &retColumns, ISpatialReference* pSROverride)
{
  CComPtr<IFields> ipFields;
  pCursor->get_Fields(&ipFields);
  for (size_t i = 0; i < fields.size(); i++)
  {
    CComPtr<IField> ipField;
    long idx = -1;
    ipFields->FindField(CComBSTR(fields[i].c_str()), &idx);
    if (idx < 0)
      continue;
    ipFields->get_Field(idx, &ipField);

    esriFieldType ft;
    ipField->get_Type(&ft);
    fl_base* f = NULL;
    switch (ft)
    {
      default: ATLASSERT(0); return NULL;
      case esriFieldTypeSmallInteger:
      case esriFieldTypeOID:
      case esriFieldTypeInteger:
        f = new fl<int>(ft, idx, fields[i].c_str());
        break;
      case esriFieldTypeSingle: 
      case esriFieldTypeDouble:
      case esriFieldTypeDate:
        f = new fl<double>(ft, idx, fields[i].c_str());
        break;
      case esriFieldTypeString:
        f = new fl<std::wstring>(ft, idx, fields[i].c_str());
        break;
      case esriFieldTypeGeometry:
      {
        //ComQIPtr<IFeatureCursor> ipFCursor(pCursor);
        //ipFCursor->
        CComPtr<IGeometryDef> ipGDef;
        ipField->get_GeometryDef(&ipGDef);
        f = new r_geometry(ft, idx, fields[i].c_str(), ipGDef, pSROverride);
      }break;
    }
    retColumns.push_back(f);
  }

  HRESULT hr = S_OK;
  CComPtr<IRow> ipRow;
  while (ipRow.Release(), pCursor->NextRow(&ipRow) == S_OK && hr == S_OK)
  {
    if (isCancel())
    {
      hr = E_ABORT;
      break;
    }
    for (size_t i = 0, n = retColumns.size(); i < n; i++)
    {
      CComVariant val;
      hr = ipRow->get_Value(retColumns[i]->idx, &val);
      if (hr != S_OK)
        break;
      try
      {
        retColumns[i]->push(val);
      }catch (...)
      {
         //ATLTRACE("\n*** exception:%s", __ex__.what());
         //CComPtr<IGPMessage> ipMsg;
         //if (RInit::newMessage(__ex__.what(), &ipMsg, esriGPMessageTypeError))
         //  g_pGeoProcessor->AddReturnMessage(ipMsg);
         hr = E_OUTOFMEMORY;
         break;
      }
    }
  }
  return hr;
}
