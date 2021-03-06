#include "stdafx.h"
#include "resource.h"
#include "OverViewOrderGrid.h"
#include "GridTradeAndOrder.h"
#include <afxdisp.h>
#include <time.h>
#include <locale.h>
#include "MainFrm.h"
#include "document.h"
#include <iostream>
#include "CNetPosTable.h"
#include "Netpos_Get_Data.h"
#include "CTrade_Table.h"
using namespace std;
using namespace rapidjson;
#import "C:\Program Files\Common Files\System\ADO\msado15.dll" \
no_namespace rename("EOF", "EndOfFile")

#define WM_MY_THREAD_MESSAGE1				WM_APP+200
#define WM_MY_THREAD_MESSAGE_ROWSNO1		WM_APP+201
#define WM_MY_THREAD_MESSAGE_REFRESH1		WM_APP+202
#define GRID_ROWS_COUNT1					WM_APP+203
#define DELETE_ROW1	     					WM_APP+204


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//CUIThread
BEGIN_MESSAGE_MAP(OverViewOrderGrid,CUGCtrl)	
	ON_WM_SIZE()
	ON_WM_TIMER()
//	ON_MESSAGE(WM_MY_THREAD_MESSAGE1, OnThreadMessage)
//	ON_MESSAGE(WM_MY_THREAD_MESSAGE_ROWSNO1, OnThreadMessage_RowsNo)		
//	ON_MESSAGE(WM_MY_THREAD_MESSAGE_REFRESH1, GridRefresh)		
//	ON_MESSAGE(GRID_ROWS_COUNT1, GridRowCount)		
//	ON_MESSAGE(DELETE_ROW1, RowDelete)	
	//ON_MESSAGE(DELETE_THREAD,DeleteThred)
END_MESSAGE_MAP()
//Structure variable
OverViewOrderGrid::st_OverviewOrder_Array OverViewOrderGrid::m_OverviewOrder_Array_Fill;
OverViewOrderGrid::st_OverviewOrder_Array OverViewOrderGrid::m_OverviewOrder_Grid_array;
OverViewOrderGrid::st_OverviewOrder_Array OverViewOrderGrid::m_OverviewOrder_Array;
CMutex OverViewOrderGrid::OverviewOrder_mutex;


int OverViewOrderGrid::filter_break=0;
int OverViewOrderGrid::insertFilterFlag=0;
int OverViewOrderGrid::check_First=0;
_variant_t OverViewOrderGrid::avarRecords;
int OverViewOrderGrid::lock_data=0;
int OverViewOrderGrid::intRows=0;
int OverViewOrderGrid::lock_data_fill=0;
int OverViewOrderGrid::Grid_Rows_Count=0;
int OverViewOrderGrid::data_display=0;
int OverViewOrderGrid::thred_kill=0;
int OverViewOrderGrid::thred_killed_ok=0;

int OverViewOrderGrid::OverViewOrderGrid_terminate=0;

//filter value global use
CString OverViewOrderGrid::col0_val=L"";
CString OverViewOrderGrid::col1_val=L"";
CString OverViewOrderGrid::col2_val=L"";
CString OverViewOrderGrid::col3_val=L"";
CString OverViewOrderGrid::col4_val=L"";
CString OverViewOrderGrid::col5_val=L"";
CString OverViewOrderGrid::col6_val=L"";
CString OverViewOrderGrid::col7_val=L"";
CString OverViewOrderGrid::col8_val=L"";
CString OverViewOrderGrid::col9_val=L"";

//_bstr_t OverViewOrderGrid::m_selected_login="0";
//_bstr_t OverViewOrderGrid::m_selected_Name="0";
_bstr_t OverViewOrderGrid::strShort("  order by t1.login asc,t1.symbol asc");
_bstr_t OverViewOrderGrid::cellvalue("");
_bstr_t OverViewOrderGrid::strFilter(" ");
_variant_t OverViewOrderGrid::result;
long OverViewOrderGrid::rgIndices[2];
_bstr_t OverViewOrderGrid::bstr_currenttime("");
/////////////////////////////////////////////////////////////////////////////
//Standard MyCug construction/destruction
int OverViewOrderGrid_trhread=0;
OverViewOrderGrid::OverViewOrderGrid()
{
	 t = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
	 bstr_currenttime=t;
	UGXPThemes::UseThemes(false);
	OverViewOrderGrid_trhread=0;
}


OverViewOrderGrid::~OverViewOrderGrid()
{
	try
	{
		OverViewOrderGrid_trhread=1;
	UGXPThemes::CleanUp();
	
	//OverViewOrderGrid::thred_kill=1;
	DWORD exit_code= NULL;
	//ExitThread();
	if (m_pThreads != NULL)
	{
	//m_pThreads->SuspendThread();
	
	if(WaitForSingleObject(m_pThreads->m_hThread,INFINITE) == WAIT_OBJECT_0) 
	{
		GetExitCodeThread(m_pThreads->m_hThread, &exit_code);
		if(exit_code == STILL_ACTIVE)
		{
			::TerminateThread(m_pThreads->m_hThread, 0);
			CloseHandle(m_pThreads->m_hThread);
		}

		
			m_pThreads->m_hThread = NULL;
			m_pThreads = NULL;
	}
	}
	}
	catch(_com_error & ce)
	{
		AfxMessageBox(ce.Description()+L"Thread UnInitiliaze");			
	}
}

UINT update_data_Order(void*);
UINT update_data_Order(void *pParam)
{
	HRESULT hr;
	OverViewOrderGrid* pThis= (OverViewOrderGrid*)pParam;	
	CoInitialize(NULL);
	CDataSource connection;
	CSession session;
	CCommand<CAccessor<CTrade_Table> > artists1;	

	OverViewOrderGrid::OverViewOrderGrid_terminate=0;

	hr=connection.OpenFromInitializationString(L"Provider=SQLNCLI11.1;Password=ok@12345;Persist Security Info=False;User ID=sa;Initial Catalog=TradeDataBase;Data Source=64.251.7.161;Use Procedure for Prepare=1;Auto Translate=True;Packet Size=4096;Workstation ID=WINDOWS-LOJSHQK;Initial File Name=\"\";Use Encryption for Data=False;Tag with column collation when possible=False;MARS Connection=False;DataTypeCompatibility=0;Trust Server Certificate=False;Application Intent=READWRITE");

	if(SUCCEEDED(hr))
     {
		hr=session.Open(connection);
		while (OverViewOrderGrid_trhread==0)
		{				
			_bstr_t strCommand="";		
			_bstr_t str_maincode="";
			_bstr_t str_comment="";

			CString login =GridTradeAndOrder::m_selected_login;
			CString sel_comment=login.Mid(7,login.GetLength()-7);
            login=login.Mid(0,6);
			str_maincode=login;
			str_comment=sel_comment;

			strCommand="select Symbol,[Order],[timeSetup],case when (Tradedatabase.dbo.mt5_orders.[Type]=2 ) then 'BUY LIMIT'  when (Tradedatabase.dbo.mt5_orders.[Type]=3 ) then 'SELL LIMIT' when (Tradedatabase.dbo.mt5_orders.[Type]=4 ) then 'BUY STOP' when (Tradedatabase.dbo.mt5_orders.[Type]=5 ) then 'SELL STOP' end as  'TYPE',volumeInitial/10000 as  'volume',cast(round(cast(PriceOrder as decimal(20,4)),4) as float) as Price,'' as Current_rate,''  as 'PL','Pending'   as 'Status',''   as 'TradeStatus' from Tradedatabase.dbo.mt5_orders left outer join client on client.v_login=cast(Tradedatabase.dbo.mt5_orders.[login] as varchar(20)) where  (cast(Tradedatabase.dbo.mt5_orders.[login] as varchar(50))='" + str_maincode + "' and comment='" + str_comment + "' and isnull(Comment_YN,'N') ='Y')  or (cast(Tradedatabase.dbo.mt5_orders.[login] as varchar(50))='" + str_maincode + "' and isnull(Comment_YN,'N') <>'Y')";
		
			 if(SUCCEEDED(hr))
			 {
				 hr=artists1.Open(session,(LPCTSTR)strCommand);							 
			 }
			 		
			 if(SUCCEEDED(hr))
			 {
				 OverViewOrderGrid::m_OverviewOrder_Array_Fill.Clear();
				 OverViewOrderGrid::st_OverviewOrder m_st_OverviewOrder={};		
				 while (artists1.MoveNext() == S_OK)
				 {									
					LPTSTR endPtr;
				    double d_m_PL = _tcstod(artists1.m_Price, &endPtr);												
					CString str_price=L"";
					str_price.Format(_T("%.4f"),d_m_PL);	
					CString str_Current_rate=L"";
					str_Current_rate.Format(_T("%.4f"),artists1.m_Current_Rate);	

					CMTStr::Copy(m_st_OverviewOrder.Symbol ,artists1.m_Symbol );				 					
					CMTStr::Copy(m_st_OverviewOrder.Order ,artists1.m_Order );				 												
					CMTStr::Copy(m_st_OverviewOrder.Time,artists1.m_Time ) ;
					CMTStr::Copy(m_st_OverviewOrder.Type ,artists1.m_Type );		
					CMTStr::Copy(m_st_OverviewOrder.Volume ,artists1.m_Volume );
					CMTStr::Copy(m_st_OverviewOrder.Price,str_price);
					CMTStr::Copy(m_st_OverviewOrder.Current_Rate ,str_Current_rate);		
					CMTStr::Copy(m_st_OverviewOrder.PL ,artists1.m_PL );
					CMTStr::Copy(m_st_OverviewOrder.Status,artists1.m_Status);
					CMTStr::Copy(m_st_OverviewOrder.Trade_Checked,artists1.m_Trade_Checked);

					OverViewOrderGrid::m_OverviewOrder_Array_Fill.Add(&m_st_OverviewOrder);
				 }
				 artists1.Close();				    									 			 				 
			 }

			 OverViewOrderGrid::OverviewOrder_mutex.Lock();	

			  //COLUMN VALUES FILTERATION
			  int val_type=0;	
				 val_type=0;
				 if (OverViewOrderGrid::insertFilterFlag==1 )
				 {
					 OverViewOrderGrid::m_OverviewOrder_Grid_array.Clear();
					 int noof_rowsInStruc=OverViewOrderGrid::m_OverviewOrder_Array_Fill.Total();
					for(int fcount=0;fcount<noof_rowsInStruc;fcount++)
					{
						OverViewOrderGrid::st_OverviewOrder m_st_Netposition={};
						m_st_Netposition=OverViewOrderGrid::m_OverviewOrder_Array_Fill[fcount];
						int flag=0;				
						CString col_row_val[9];		
						col_row_val[0]=m_st_Netposition.Symbol;
						if (OverViewOrderGrid::col0_val.Trim().GetLength()>0)
						{
							col_row_val[0]=col_row_val[0].Mid(0,OverViewOrderGrid::col0_val.Trim().GetLength());
						}
						col_row_val[1]=m_st_Netposition.Order;
						if (OverViewOrderGrid::col1_val.Trim().GetLength()>0)
						{
							col_row_val[1]=col_row_val[1].Mid(0,OverViewOrderGrid::col1_val.Trim().GetLength());
							col_row_val[1]=col_row_val[1].Mid(0,10);
						}
						col_row_val[2]=m_st_Netposition.Time ;
						if (OverViewOrderGrid::col2_val.Trim().GetLength()>0)
						{
							col_row_val[2]=col_row_val[2].Mid(0,OverViewOrderGrid::col2_val.Trim().GetLength());
						}
						
						col_row_val[3]=m_st_Netposition.Type ;
						if (OverViewOrderGrid::col3_val.Trim().GetLength()>0)
						{
							col_row_val[3]=col_row_val[3].Mid(0,OverViewOrderGrid::col3_val.Trim().GetLength());
						}

						col_row_val[4]=m_st_Netposition.Volume;
						if (OverViewOrderGrid::col4_val.Trim().GetLength()>0)
						{
							col_row_val[4]=col_row_val[4].Mid(0,OverViewOrderGrid::col4_val.Trim().GetLength());
						}

						col_row_val[5]=m_st_Netposition.Price;;
						boolean bool_col5=Check_numeric_col_filter(OverViewOrderGrid::col5_val,col_row_val[5]);

						col_row_val[6]=m_st_Netposition.Current_Rate;;
						boolean bool_col6=Check_numeric_col_filter(OverViewOrderGrid::col6_val,col_row_val[6]);

						col_row_val[7]=m_st_Netposition.PL;
						boolean bool_col7=Check_numeric_col_filter(OverViewOrderGrid::col7_val,col_row_val[7]);

						col_row_val[8]=m_st_Netposition.Status;
						if (OverViewOrderGrid::col8_val.Trim().GetLength()>0)
						{
							col_row_val[8]=col_row_val[8].Mid(0,OverViewOrderGrid::col8_val.Trim().GetLength());
						}

						/*col_row_val[9]=m_st_Netposition.Trade_Checked ;
						if (OverViewOrderGrid::col9_val.Trim().GetLength()>0)
						{
							col_row_val[9]=col_row_val[9].Mid(0,OverViewOrderGrid::col9_val.Trim().GetLength());
						}*/

						if((OverViewOrderGrid::col0_val.Trim()==col_row_val[0].Trim() || OverViewOrderGrid::col0_val.Trim()==L"ALL"||OverViewOrderGrid::col0_val.Trim()==L"") && (OverViewOrderGrid::col1_val.Trim()==col_row_val[1].Trim() || OverViewOrderGrid::col1_val.Trim()==L"ALL"||OverViewOrderGrid::col1_val.Trim()==L"") && (OverViewOrderGrid::col2_val.Trim()==col_row_val[2].Trim() || OverViewOrderGrid::col2_val.Trim()==L"ALL"||OverViewOrderGrid::col2_val.Trim()==L"")  && (OverViewOrderGrid::col3_val.Trim()==col_row_val[3].Trim() || OverViewOrderGrid::col3_val.Trim()==L"ALL"||OverViewOrderGrid::col3_val.Trim()==L"") && (OverViewOrderGrid::col4_val.Trim()==col_row_val[4].Trim() || OverViewOrderGrid::col4_val.Trim()==L"ALL"||OverViewOrderGrid::col4_val.Trim()==L"")   && (bool_col5==true || OverViewOrderGrid::col5_val.Trim()==L"ALL"||OverViewOrderGrid::col5_val.Trim()==L"")   && (bool_col6==true || OverViewOrderGrid::col6_val.Trim()==L"ALL"||OverViewOrderGrid::col6_val.Trim()==L"")   && (bool_col7==true || OverViewOrderGrid::col7_val.Trim()==L"ALL"||OverViewOrderGrid::col7_val.Trim()==L"")   && (OverViewOrderGrid::col8_val.Trim()==col_row_val[8].Trim() || OverViewOrderGrid::col8_val.Trim()==L"ALL"||OverViewOrderGrid::col8_val.Trim()==L"")/* && (OverViewOrderGrid::col9_val.Trim()==col_row_val[9].Trim() || OverViewOrderGrid::col9_val.Trim()==L"ALL"||OverViewOrderGrid::col9_val.Trim()==L"")*/)
				        {
							 OverViewOrderGrid::m_OverviewOrder_Grid_array.Add(&m_st_Netposition);
						}
					}
				 }
				 else
				 {
					 OverViewOrderGrid::m_OverviewOrder_Grid_array.Assign(OverViewOrderGrid::m_OverviewOrder_Array_Fill);
				 }










			
			 OverViewOrderGrid::OverviewOrder_mutex.Unlock();	
			 Sleep(1000);
		 }
	  }
	session.Close();
	connection.Close();
	OverViewOrderGrid::OverViewOrderGrid_terminate=1;
    return 0;
}



LRESULT OverViewOrderGrid::GridRefresh(WPARAM wParam, LPARAM lParam)
{
	if (OverViewOrderGrid::thred_kill==0)
	{
		RedrawAll();
	}
	return 0;
}



LRESULT OverViewOrderGrid::OnThreadMessage(WPARAM wParam, LPARAM lParam)
{
	if (OverViewOrderGrid::thred_kill==0)
	{
	int col= (int)wParam;
	int row= (int)lParam;
	
	if (OverViewOrderGrid::insertFilterFlag==0)
	{
		
		QuickSetText(col,row,OverViewOrderGrid::cellvalue);	
		
		//RedrawCell(col,row);
	}
	else
	{
		QuickSetText(col,row+1,OverViewOrderGrid::cellvalue);	
			
		//RedrawCell(col,row+1);
	}
	}
	return 0;
}

LRESULT OverViewOrderGrid::GridRowCount(WPARAM wParam, LPARAM lParam)
{
	//RedrawAll();
	if (OverViewOrderGrid::thred_kill==0)
	{
		OverViewOrderGrid::Grid_Rows_Count=GetNumberRows();
	}
	return 0;
}

LRESULT OverViewOrderGrid::RowDelete(WPARAM wParam, LPARAM lParam)
{
	if (OverViewOrderGrid::thred_kill==0)
	{
	
	int row= (int)wParam;
	for(int f=0;f<10;f++)
	{
		QuickSetText(f,row,L"");
	}
	//DeleteRow(row);
	}
	return 0;
}

LRESULT OverViewOrderGrid::DeleteThred(WPARAM wParam, LPARAM lParam)
{
	/*DWORD exit_code= NULL;
	if (m_pThreads != NULL)
	{
    GetExitCodeThread(m_pThreads->m_hThread, &exit_code);
    if(exit_code == STILL_ACTIVE)
    {
        ::TerminateThread(m_pThreads->m_hThread, 0);
        CloseHandle(m_pThreads->m_hThread);
    }
    m_pThreads->m_hThread = NULL;
    m_pThreads = NULL;
	}*/

	
	return 0;
}




LRESULT OverViewOrderGrid::OnThreadMessage_RowsNo(WPARAM wParam, LPARAM lParam)
{
	if (OverViewOrderGrid::thred_kill==0)
	{
	int val= (int)wParam;
	int flag= (int)lParam;
	if(flag==0)
	{
		SetNumberRows(val);
	}
	if(flag==1)
	{
		InsertRow(val);
	}
	}
		return 0;
}







/////////////////////////////////////////////////////////////////////////////
//	OnSheetSetup	
//		This notification is called for each additional sheet that the grid
//		might contain, here you can customize each sheet in the grid.
//	Params:
//		sheetNumber - idndex of current sheet
//	Return:
//		<none>
void OverViewOrderGrid::OnSheetSetup(int sheetNumber)
{
	int	nRow = 0, nCol = 0;
	// ****************************************************************
	// ** Set up columns
	// ****************************************************************
	CUGCell cell;

	GetGridDefault(&cell);
	//cell.SetBackColor(MY_COLOR_BACK);
	//cell.SetTextColor(MY_COLOR_TEXT);
	SetGridDefault(&cell);
	EnableExcelBorders(TRUE);
	SetHighlightRow(TRUE, FALSE);
	SetDoubleBufferMode(TRUE);
	SetDefFont(0);
	SetSH_Width(0);
	
	SetRowHeight(-1,0);
	//// Number
			SetNumberCols(13);
			QuickSetText(0,-1,L"Symbol");
			SetColWidth(0,100);
			QuickSetText(1,-1,L"Order");
			SetColWidth(1,50);
			QuickSetText(2,-1,L"Time");	
			SetColWidth(2,140);
			
			QuickSetText(3,-1,L"Type");
			SetColWidth(3,60);
			
			QuickSetText(4,-1,L"Volume");
			SetColWidth(4,40);
			QuickSetText(5,-1,L"Price");	
			SetColWidth(5,60);
			QuickSetText(6,-1,L"LTP Price");
			SetColWidth(6,60);
			QuickSetText(7,-1,L"P&L");
			SetColWidth(7,70);
			
			QuickSetText(8,-1,L"Status");
			SetColWidth(8,60);		

			QuickSetText(9,-1,L"Checked Status");
			SetColWidth(9,80);		

			QuickSetText(10,-1,L"Checked Time");
			SetColWidth(10,130);

			QuickSetText(11,-1,L"Remark1");
			SetColWidth(11,100);		

			QuickSetText(12,-1,L"Remark1");
			SetColWidth(12,100);		
			
	// Header font
	for(int i = 0; i < GetNumberCols(); i++)
	{
		QuickSetFont(i, -1, 1);
	}	
}

		

void OverViewOrderGrid::OnDClicked(int col,long row,RECT *rect,POINT *point,BOOL processed)
{

	
	//CDialog mySettings( IDD_TAB_DATA );
   //INT_PTR returnCode = -1;
   //returnCode = mySettings.DoModal();

   
	CUGCell cell;
	GetCell(col,row,&cell);
	int nCellTypeIndex = cell.GetCellType();
	int nParam = cell.GetParam();
	CString string;
	if(cell.GetLabelText() != NULL)
		string = cell.GetLabelText();

	if(processed){
		if(cell.GetCellType() == m_nSpinIndex)
			return ;
	}

	if(nParam == CELLTYPE_IS_EDITABLE || string == "CELLTYPE_IS_EDITABLE")
	{
		StartEdit();
	}
}	
			 

void OverViewOrderGrid::OnTH_LClicked(int col,long row,int updn,RECT *rect,POINT *point,BOOL processed)
{	
	UNREFERENCED_PARAMETER(row);
	UNREFERENCED_PARAMETER(rect);
	UNREFERENCED_PARAMETER(point);
	UNREFERENCED_PARAMETER(processed);

	if( updn == 0)
		return;

	QuickSetCellType( m_iSortCol, -1, 0 );

	if( col == m_iSortCol )
	{
		if( m_bSortedAscending )
			m_bSortedAscending = FALSE;
		else
			m_bSortedAscending = TRUE;
	}
	else
	{
		m_iSortCol = col;
		m_bSortedAscending = TRUE;
	}

	if( m_bSortedAscending )
	{
		SortBy( col, UG_SORT_ASCENDING );
		QuickSetCellType( m_iSortCol, -1, m_iArrowIndex );
		QuickSetCellTypeEx( m_iSortCol, -1, UGCT_SORTARROWDOWN );

//		((CFindAndSortDemo*)GetParent())->Trace( _T( "Sorted column %d ascending" ), iCol );
	}
	else
	{
		SortBy( col, UG_SORT_DESCENDING );
		QuickSetCellType( m_iSortCol, -1, m_iArrowIndex );
		QuickSetCellTypeEx( m_iSortCol, -1, UGCT_SORTARROWUP );
//		Trace( _T( "Sorted column %d descending" ), iCol );
	}
	
	RedrawAll();
}
int OverViewOrderGrid::OnCellTypeNotify(long ID,int col,long row,long msg,long param)
{

	if(ID == UGCT_DROPLIST){
		return OnDropList(ID, col, row, msg, param);
	}
}
int OverViewOrderGrid::OnDropList(long ID,int col,long row,long msg,long param)
{
	if (msg==103)
	{
		if(OverViewOrderGrid::insertFilterFlag==1 && row==0)
		{
			OverViewOrderGrid::filter_break=1;
			check_First==0;
			CString  strval=L"";
			CUGCell cell;
			GetCell(col,row,&cell);
			strval=cell.GetText();
			OverViewOrderGrid::strFilter="";

		}

		if(OverViewOrderGrid::insertFilterFlag==1 && row==0 )
		{
			
			CString  strval=L"";
			CUGCell cell;
			GetCell(col,row,&cell);
			strval=cell.GetText();	
			if(col==0)
			{
				if (strval!=L"")
				{
					OverViewOrderGrid::col0_val=strval;					
				}
				else
				{
					OverViewOrderGrid::col0_val=L"ALL";					
				}
			}


			if(col==1)
			{
				if (strval!=L"")
				{
					OverViewOrderGrid::col1_val=strval;					
				}
				else
				{
					OverViewOrderGrid::col1_val=L"ALL";					
				}
			}

			if(col==2)
			{
				if (strval!=L"")
				{
					OverViewOrderGrid::col2_val=strval;					
				}
				else
				{
					OverViewOrderGrid::col2_val=L"ALL";					
				}
			}

			if(col==3)
			{
				if (strval!=L"")
				{
					OverViewOrderGrid::col3_val=strval;					
				}
				else
				{
					OverViewOrderGrid::col3_val=L"ALL";					
				}
			}

			if(col==4)
			{
				if (strval!=L"")
				{
					OverViewOrderGrid::col4_val=strval;					
				}
				else
				{
					OverViewOrderGrid::col4_val=L"ALL";					
				}
			}

			if(col==5)
			{
				if (strval!=L"")
				{
					OverViewOrderGrid::col5_val=strval;					
				}
				else
				{
					OverViewOrderGrid::col5_val=L"ALL";					
				}
			}

			if(col==6)
			{
				if (strval!=L"")
				{
					OverViewOrderGrid::col6_val=strval;					
				}
				else
				{
					OverViewOrderGrid::col6_val=L"ALL";					
				}
			}

			if(col==7)
			{
				if (strval!=L"")
				{
					OverViewOrderGrid::col7_val=strval;					
				}
				else
				{
					OverViewOrderGrid::col7_val=L"ALL";					
				}
			}

			if(col==8)
			{
				if (strval!=L"")
				{
					OverViewOrderGrid::col8_val=strval;					
				}
				else
				{
					OverViewOrderGrid::col8_val=L"ALL";					
				}
			}

			if(col==9)
			{
				if (strval!=L"")
				{
					OverViewOrderGrid::col9_val=strval;					
				}
				else
				{
					OverViewOrderGrid::col9_val=L"ALL";					
				}
			}
         
        }
	  RedrawAll();
	 }
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//	OnEditFinish
//		This notification is sent when the edit is being finised
//	Params:
//		col, row	- coordinates of the edit cell
//		edit		- pointer to the edit control
//		string		- actual string that user typed in
//		cancelFlag	- indicates if the edit is being cancelled
//	Return:
//		TRUE - to allow the edit it proceede
//		FALSE - to force the user back to editing of that same cell

_bstr_t OverViewOrderGrid::get_string(CString  MainStr,CString SepStr)
{
	int str_len=MainStr.GetLength();
	CString strcode=L"";	
	_bstr_t b_strreturn="";
	_bstr_t b_strCode="";
	_bstr_t b_strChar="";
	_bstr_t bstrcomp="";
	for(int forcount=0;forcount<str_len;forcount++)
	{
		CString strchar=MainStr.Mid(forcount,1);
		if (strchar!=SepStr)
		{
			b_strChar=strchar;
			b_strCode=b_strCode+b_strChar;
		}
		else
		{
			if(b_strreturn!=bstrcomp)
			{
				b_strreturn=b_strreturn+","+"'"+b_strCode+"'";
			}
			else
			{
				b_strreturn="'"+b_strCode+"'";
			}
			b_strCode="";
		}
	}
	
	if (b_strreturn!=bstrcomp)
	{
		b_strreturn=b_strreturn+","+"'"+b_strCode+"'";
	}
	else
	{
		b_strreturn="'" +b_strCode +"'";
	}

	b_strreturn="("+b_strreturn+")";
	return b_strreturn;

}



int OverViewOrderGrid::OnEditFinish(int col, long row,CWnd *edit,LPCTSTR string,BOOL cancelFlag)
{
 if(OverViewOrderGrid::insertFilterFlag==1 && row==0)
	{
		OverViewOrderGrid::filter_break=1;
		check_First==0;
		CString  strval=L"";
		CUGCell cell;
		QuickSetText(col,row,string );
		GetCell(col,row,&cell);
		strval=cell.GetText();
		OverViewOrderGrid::strFilter="";
		if (strval!=L"")
		{
			//gridFilter(col,GetNumberRows(),strval);
		}
	}
	return TRUE;
	
}


/////////////////////////////////////////////////////////////////////////////
//	OnMenuCommand
//		This notification is called when the user has selected a menu item
//		in the pop-up menu.
//	Params:
//		col, row - the cell coordinates of where the menu originated from
//		setcion - identify for which portion of the gird the menu is for.
//				  possible sections:
//						UG_TOPHEADING, UG_SIDEHEADING,UG_GRID
//						UG_HSCROLL  UG_VSCROLL  UG_CORNERBUTTON
//		item - ID of the menu item selected
//	Return:
//		<none>
void OverViewOrderGrid::OnMenuCommand(int col,long row,int section,int item)
{
	UNREFERENCED_PARAMETER(col);
	UNREFERENCED_PARAMETER(row);
	UNREFERENCED_PARAMETER(section);
	// init. local variables

	switch( item )
	{
	
		case 2001:
		{
			filter();
//			Trace( _T( "Displayed the grid's Replace Dialog." ) );
			break;
		}
		
	}
}




/////////////////////////////////////////////////////////////////////////////
//	OnSortEvaluate
//		Sent as a result of the 'SortBy' call, this is called for each cell
//		comparison and allows for customization of the sorting routines.
//		We provide following code as example of what could be done here,
//		you migh have to modify it to give your application customized sorting.
//	Params:
//		cell1, cell2	- pointers to cells that are compared
//		flags			- identifies sort direction
//	Return:
//		value less than zero to identify that the cell1 comes before cell2
//		value equal to zero to identify that the cell1 and cell2 are equal
//		value greater than zero to identify that the cell1 comes after cell2
int OverViewOrderGrid::OnSortEvaluate(CUGCell *cell1,CUGCell *cell2,int flags)
{
	//if( flags & UG_SORT_DESCENDING )
	//{
	//	CUGCell *ptr = cell1;
	//	cell1 = cell2;
	//	cell2 = ptr;
	//}

	//COLORREF cr1 = cell1->GetBackColor();
	//COLORREF cr2 = cell2->GetBackColor();
	//COleDateTime date1, date2;
	//CString str1, str2;
	//int iLen1, iLen2;
	//int retVal = 0;

	//BYTE color1, color2;

	//switch ( m_iSortCol )
	//{
	//case 5:
	//	color1 = GetRValue( cr1 );
	//	color2 = GetRValue( cr2 );

	//	if( color1 > color2 )
	//		retVal = -1;
	//	if( color1 < color2 )
	//		retVal = 1;

	//	break;
	//case 6:
	//	color1 = GetGValue( cr1 );
	//	color2 = GetGValue( cr2 );

	//	if( color1 > color2 )
	//		retVal = -1;
	//	if( color1 < color2 )
	//		retVal = 1;
	//
	//	break;

	//case 7:
	//	color1 = GetBValue( cr1 );
	//	color2 = GetBValue( cr2 );

	//	if( color1 > color2 )
	//		retVal = -1;
	//	if( color1 < color2 )
	//		retVal = 1;
	//
	//	break;

	//case 8:
	//	str1 = cell1->GetText();
	//	str2 = cell2->GetText();
	//	iLen1 = str1.GetLength();
	//	iLen2 = str2.GetLength();

	//	if( iLen1 < iLen2 )
	//		retVal = -1;
	//	if( iLen1 > iLen2 )
	//		retVal = 1;

	//	break;

	//case 9:
	//	date1.ParseDateTime( cell1->GetText(), VAR_DATEVALUEONLY );
	//	date2.ParseDateTime( cell2->GetText(), VAR_DATEVALUEONLY );

	//	// If a date is invalid, move it to the bottom, by always making it >
	//	// ( the valid code is 0 )
	//	if (date1.GetStatus())
	//		return 1;
	//	if (date2.GetStatus())
	//		return -1;
	//	
	//	if( date1 < date2 )
	//		retVal = -1;
	//	if( date1 > date2 )
	//		retVal = 1;

	//	break;

	//default:
	//	if( CString(cell1->GetText()) == "" )
	//		return 1;
	//	else if( CString(cell2->GetText()) == "" )
	//		return -1;

	//	switch( cell1->GetDataType() )
	//	{
	//		case UGCELLDATA_NUMBER:
	//		case UGCELLDATA_BOOL:
	//		case UGCELLDATA_CURRENCY:
	//			if( cell1->GetNumber() < cell2->GetNumber())
	//				retVal = -1;
	//			if( cell1->GetNumber() > cell2->GetNumber())
	//				retVal = 1;

	//			break;

	//		default:
	//			retVal = _tcscmp( cell1->GetText( ), cell2->GetText());
	//	}
	//}

	//return retVal;
return 0;
}

/////////////////////////////////////////////////////////////////////////////
//	OnTabSelected
//		Called when the user selects one of the tabs on the bottom of the screen
//	Params:
//		ID	- id of selected tab
//	Return:
//		<none>
void OverViewOrderGrid::OnTabSelected(int ID)
{
	SetSheetNumber(ID);
	PostMessage(WM_SIZE, 0, 0L);
}

/////////////////////////////////////////////////////////////////////////////
//	OnAdjustComponentSizes
//		Called when the grid components are baing arranged.
//	Params:
//		grid, topHdg, sideHdg, cnrBtn, vScroll, hScroll, 
//		tabs	- sizes and location of each of the grid components
//	Return:
//		<none>

void OverViewOrderGrid::OnSize(UINT nType, int cx, int cy)
{
	/*EnableUpdate(FALSE);

	RECT rect;
	GetClientRect(&rect);

	SetTabWidth(max(rect.right/3, 75));
	SetSH_Width(0);
	SetColWidth(0, 175);
	for(int y = 1;y < 14; y++)
	{
		SetColWidth(y, (rect.right-175)/6);
	}

	EnableUpdate(TRUE);


	CUGCtrl::OnSize(nType,cx,cy);*/
}



void OverViewOrderGrid::OnTimer(UINT nIDEvent)
{		
	OverViewOrderGrid::OverviewOrder_mutex.Lock();
	int r_count=OverViewOrderGrid::m_OverviewOrder_Grid_array.Total();
	
	int grid_total=GetNumberRows();
	if (OverViewOrderGrid::insertFilterFlag==1)
	{
		r_count=r_count+1;
	}		
	if (grid_total!=r_count)
	{			
		SetNumberRows(r_count);					
	}
	else
	{			
		RedrawAll();			
	}		

	OverViewOrderGrid::OverviewOrder_mutex.Unlock();

}





 void OverViewOrderGrid::InitMenu()
{
	EnableMenu(TRUE);
	
	
	AddMenuItem( 2001, _T( "Filter" ) );
	
	
}

 int OverViewOrderGrid::OnMenuStart( int iCol, long lRow, int iSection )
{
	UNREFERENCED_PARAMETER(iCol);
	UNREFERENCED_PARAMETER(lRow);

	if( iSection == UG_GRID )
	{
//		Trace( _T( "Displayed the grid's internal pop-up menu." ) );	
		return TRUE;
	}
	else
		return FALSE;
}

 void OverViewOrderGrid::filter()
 {
	if (OverViewOrderGrid::insertFilterFlag==0)
	{	
			OverViewOrderGrid::insertFilterFlag=1;
			addItemToCombobox();
		    InsertRow(0);
			for (int col_count=0;col_count<10;col_count++)
			{
				CUGCell cell;
				int row=0;
				int col=col_count;
				cell.ClearAll();
				cell.SetCellType(UGCT_DROPLIST);
				cell.SetCellTypeEx(UGCT_DROPLISTHIDEBUTTON);
				cell.SetParam(CELLTYPE_IS_EDITABLE);
				QuickSetRange(col,row,col,row,&cell);
				QuickSetLabelText(col,row,str[col_count]);
			}
	}
	else
	{
		
		DeleteRow(0);
		OverViewOrderGrid::insertFilterFlag=0;
		OverViewOrderGrid::strFilter=" ";
		OverViewOrderGrid::col0_val=L"";
		OverViewOrderGrid::col1_val=L"";
		OverViewOrderGrid::col2_val=L"";
		OverViewOrderGrid::col3_val=L"";
		OverViewOrderGrid::col4_val=L"";
		OverViewOrderGrid::col5_val=L"";
		OverViewOrderGrid::col6_val=L"";
		OverViewOrderGrid::col7_val=L"";
		OverViewOrderGrid::col8_val=L"";
		OverViewOrderGrid::col9_val=L"";
	}
	RedrawAll();
 }



BOOLEAN  OverViewOrderGrid::CheckvalueInArray(const CStringArray& arr,CString strval) 
{
	try
	{
	int arrcount=arr.GetCount();
    for (int forcount=0;forcount<arrcount;forcount++)
	{
		if (arr[forcount]==strval)
		{
			return true;
		}
	}
    return false  ;
	}
	catch(_com_error & ce)
			{
				AfxMessageBox(ce.Description()+L"CheckvalueInArray");			
			} 
}
 
void OverViewOrderGrid::addItemToCombobox()
{
	CStringArray arr;
	CStringArray arr1;
	CStringArray arr2;
	CStringArray arr3;
	CStringArray arr4;
	CStringArray arr5;
	CStringArray arr6;
	CStringArray arr7;
	CStringArray arr8;
	CStringArray arr9;
	try
	{
	int rows=1;
	
	
	CString str_val=L"";
	for (int forcount=0;forcount<10;forcount++)
	{
		str[forcount]=L"ALL\n";		
	}
	OverViewOrderGrid::OverviewOrder_mutex.Lock();	
	st_OverviewOrder_Array m_array_filter;
	m_array_filter.Assign(OverViewOrderGrid::m_OverviewOrder_Array_Fill);
	OverViewOrderGrid::OverviewOrder_mutex.Unlock();	

	rows=m_array_filter.Total();

	for (int forcount=0;forcount<rows;forcount++)
	{

		st_OverviewOrder m_st_for_filter={};
		m_st_for_filter=m_array_filter[forcount];


		for (int clocount=0;clocount<9;clocount++)
		{

			  if (clocount==0)
				{
					str_val=m_st_for_filter.Symbol;
					str_val=str_val.Trim();
					if (CheckvalueInArray(arr,str_val)==false && !str_val.IsEmpty())
					{
						str[clocount]=str[clocount]+str_val+L"\n";										
						arr.Add(str_val);
					}
				}


				if (clocount==1)
				{
					str_val=m_st_for_filter.Order;
					str_val=str_val.Trim();
					if (CheckvalueInArray(arr1,str_val)==false && !str_val.IsEmpty() )
					{
						str[clocount]=str[clocount]+str_val+L"\n";										
						arr1.Add(str_val);
					}
				}


				if (clocount==2)
				{
					str_val=m_st_for_filter.Time ;
					if (str_val.GetLength()>10)
					{
						str_val=str_val.Mid(0,10);
					}

					if (CheckvalueInArray(arr2,str_val)==false && !str_val.IsEmpty())
					{
						str[clocount]=str[clocount]+str_val+L"\n";									
						arr2.Add(str_val);
					}
				}



				if (clocount==3)
				{
					str_val=m_st_for_filter.Type;
					str_val=str_val.Trim();
					if (CheckvalueInArray(arr3,str_val)==false && !str_val.IsEmpty())
					{
						str[clocount]=str[clocount]+str_val+L"\n";										
						arr3.Add(str_val);
					}
				}

				if (clocount==4)
				{
					str_val=m_st_for_filter.Volume;
					str_val=str_val.Trim();
					if (CheckvalueInArray(arr4,str_val)==false && !str_val.IsEmpty())
					{
						str[clocount]=str[clocount]+str_val+L"\n";										
						arr4.Add(str_val);
					}
				}
				if (clocount==5)
				{
					str_val=m_st_for_filter.Price;
					str_val=str_val.Trim();
					if (CheckvalueInArray(arr5,str_val)==false && !str_val.IsEmpty())
					{
						str[clocount]=str[clocount]+str_val+L"\n";										
						arr5.Add(str_val);
					}
				}
				if (clocount==6)
				{
					str_val=m_st_for_filter.Current_Rate;
					str_val=str_val.Trim();
					if (CheckvalueInArray(arr6,str_val)==false && !str_val.IsEmpty())
					{
						str[clocount]=str[clocount]+str_val+L"\n";										
						arr6.Add(str_val);
					}
				}
				if (clocount==7)
				{
					
					str_val=m_st_for_filter.PL;
					str_val=str_val.Trim();
					if (CheckvalueInArray(arr7,str_val)==false && !str_val.IsEmpty())
					{
						str[clocount]=str[clocount]+str_val+L"\n";										
						arr7.Add(str_val);
					}
				}
				if (clocount==8)
				{
					str_val=m_st_for_filter.Status;
					str_val=str_val.Trim();
					if (CheckvalueInArray(arr8,str_val)==false && !str_val.IsEmpty())
					{
						str[clocount]=str[clocount]+str_val+L"\n";										
						arr8.Add(str_val);
					}
				}
				/*if (clocount==9)
				{
					str_val=m_st_for_filter.Trade_Checked;
					str_val=str_val.Trim();
					if (CheckvalueInArray(arr9,str_val)==false && !str_val.IsEmpty())
					{
						str[clocount]=str[clocount]+str_val+L"\n";										
						arr9.Add(str_val);
					}
				}*/

		}												
	}
	}
	catch(_com_error & ce)
			{
				AfxMessageBox(ce.Description()+L"addItemToCombobox");			

	       }
}
 

void OverViewOrderGrid::OnSetup()
{
	// Set up the Tab controls
	c1_click=0;
	c2_click=0;
	c3_click=0;
	c4_click=0;
	c5_click=0;
	c6_click=0;
	c7_click=0;
	c8_click=0;
	c9_click=0;
	c10_click=0;
	c11_click=0;
	c12_click=0;
	c13_click=0;
	c14_click=0;
	int	nIndex = 0;
	CUGCell cell;
	GetGridDefault(&cell);
	SetGridDefault(&cell);
	EnableExcelBorders(FALSE);
	SetHighlightRow(TRUE, FALSE);
	SetDoubleBufferMode(TRUE);
	SetUserSizingMode( TRUE );

	
	InitMenu();
	// Create and set default font
	AddFont(L"Arial", -12, FW_NORMAL);
	AddFont(L"Arial", -12, FW_BOLD);
	SetDefFont(0);

	


	CDC * pDC;
	
	int iHeight;
	int i, j;
	
	pDC = GetDC( );
	
	iHeight = -MulDiv( 8, GetDeviceCaps( pDC->m_hDC, LOGPIXELSY), 72 );
	m_font.CreateFont( iHeight, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0, _T( "MS Sans Serif" ) );
	
	ReleaseDC( pDC );

	SetDefFont( &m_font );
		
	GetGridDefault( &cell );
	cell.SetAlignment( UG_ALIGNCENTER | UG_ALIGNVCENTER );
	SetGridDefault( &cell );

	SetNumberCols( 14 );
	//SetNumberRows( 50 );

	m_iArrowIndex = AddCellType( &m_sortArrow );

	SetCurrentCellMode( 2 );

	UseHints( TRUE );

	SetSH_Width( 0 );
	/*SetUniformRowHeight( TRUE );*/
	






	// Start timers
	//srand( (unsigned)time( NULL ) );
	SetTimer(0, 100, NULL);
	SetTimer(1, 100, NULL);



	QuickSetCellType( 0, -1, m_iArrowIndex );
	SortBy( 0, UG_SORT_ASCENDING );

	m_iSortCol = 0;
	m_bSortedAscending = TRUE;



	m_pThreads=AfxBeginThread(update_data_Order, this);
	m_pThreads->m_bAutoDelete=false;
	
	
    run_check=0;		
}

void OverViewOrderGrid::gridFilter(int colno,int rows_count,CString col_value)
{
	CString getColvalue=L"";
	
	for(int fcount=rows_count-1;fcount>0;fcount--)
	{
		getColvalue=QuickGetText(colno,fcount);


		col_value=col_value.Trim();
		CString fcar=col_value.Mid(0,1);
		CString comp_value=col_value.Mid(1,col_value.GetLength()-1);
		if(fcar==L">"||fcar==L"<")
		{
			if (fcar==L">")
			{
				if(getColvalue>comp_value )
				{
					SetColWidth(2, 75);
					SetRowHeight(fcount, 20);
					SetSH_Width(0);
					SetTH_Height(20);
			
					//AfxMessageBox(L"Hide Row");
				}
				else
				{
					SetColWidth(2, 75);
					SetRowHeight(fcount, 0);
					SetSH_Width(0);
					SetTH_Height(20);
					//AfxMessageBox(L"UN Hide Row");
				}
			}
			if (fcar==L"<")
			{
				if(getColvalue<comp_value )
				{
					SetColWidth(2, 75);
					SetRowHeight(fcount, 20);
					SetSH_Width(0);
					SetTH_Height(20);
			
					//AfxMessageBox(L"Hide Row");
				}
				else
				{
					SetColWidth(2, 75);
					SetRowHeight(fcount, 0);
					SetSH_Width(0);
					SetTH_Height(20);
					//AfxMessageBox(L"UN Hide Row");
				}
			}
		}

		else
		{

		if(getColvalue==col_value || col_value==L"ALL")
		{
			SetColWidth(2, 75);
			SetRowHeight(fcount, 20);
			SetSH_Width(0);
			SetTH_Height(20);
			
			//AfxMessageBox(L"Hide Row");
		}
		else
		{
			SetColWidth(2, 75);
			SetRowHeight(fcount, 0);
			SetSH_Width(0);
			SetTH_Height(20);
			//AfxMessageBox(L"UN Hide Row");
		}
		}
	}
	
}

int OverViewOrderGrid::OnCanViewMove(int oldcol,long oldrow,int newcol,long newrow)
{
	UNREFERENCED_PARAMETER(oldcol);
	UNREFERENCED_PARAMETER(oldrow);
	UNREFERENCED_PARAMETER(newcol);
	UNREFERENCED_PARAMETER(newrow);
	return TRUE;
}


void OverViewOrderGrid::OnGetCell(int col,long row,CUGCell *cell)
{		
		//m_logfile_g.LogEvent(L"Start OnGetCell");
	    st_OverviewOrder mst_grid={};
		int rows_no=0;
		rows_no=row;		
		UNREFERENCED_PARAMETER(col);
		UNREFERENCED_PARAMETER(row);
		UNREFERENCED_PARAMETER(*cell);		
		if ( col >= 0 && row == -1 )
		{	
		}
		else if ( row >= 0 && col == -1 )
		{	
		}
		else if ( col >= 0 && row >= 0 )
		{
			if (OverViewOrderGrid::insertFilterFlag==1)
			{
				rows_no=row-1;				
				if (row==0)
				{
					return;
				}
			}
			if (col==0)
			{				
				mst_grid=OverViewOrderGrid::m_OverviewOrder_Grid_array[rows_no];				
				CString tmp=mst_grid.Symbol;
				if(!tmp.IsEmpty())
				{
				  CString str_get_value=cell->GetText();
				  if (wcscmp(str_get_value,tmp)!=0)
				  {
					cell->SetText(tmp);
				  }
				}
			}
			else if (col==1)
			{
				
				mst_grid=OverViewOrderGrid::m_OverviewOrder_Grid_array[rows_no];
				CString tmp=mst_grid.Order ;
				if(!tmp.IsEmpty())
				{
					CString str_get_value=cell->GetText();
					if (wcscmp(str_get_value,tmp)!=0)
					{
						cell->SetText(tmp);
					}
				}
			}
			else if (col==2)
			{				
				mst_grid=OverViewOrderGrid::m_OverviewOrder_Grid_array[rows_no];
				CString tmp=mst_grid.Time ;
				if(!tmp.IsEmpty())
				{
					CString str_get_value=cell->GetText();
					if (wcscmp(str_get_value,tmp)!=0)
					{
						cell->SetText(tmp);
					}
				}
			}
			else if (col==3)
			{			
				mst_grid=OverViewOrderGrid::m_OverviewOrder_Grid_array[rows_no];
				CString tmp=mst_grid.Type ;	
				if(!tmp.IsEmpty())
				{
					CString str_get_value=cell->GetText();
					if (wcscmp(str_get_value,tmp)!=0)
					{
						cell->SetText(tmp);
					}
				}
			}
			else if (col==4)
			{				
				mst_grid=OverViewOrderGrid::m_OverviewOrder_Grid_array[rows_no];
				CString tmp=mst_grid.Volume ;	
				if(!tmp.IsEmpty())
				{
					CString str_get_value=cell->GetText();
					if (wcscmp(str_get_value,tmp)!=0)
					{
						cell->SetText(tmp);
					}
				}
			}
			else if (col==5)
			{				
				mst_grid=OverViewOrderGrid::m_OverviewOrder_Grid_array[rows_no];
				CString tmp=mst_grid.Price  ;
				if(!tmp.IsEmpty())
				{
					CString str_get_value=cell->GetText();
					if (wcscmp(str_get_value,tmp)!=0)
					{
						cell->SetText(tmp);
					}
				}
			}
			else if (col==6)
			{	
				mst_grid=OverViewOrderGrid::m_OverviewOrder_Grid_array[rows_no];
				CString tmp=mst_grid.Current_Rate  ;
				if(!tmp.IsEmpty())
				{
					CString str_get_value=cell->GetText();
					if (wcscmp(str_get_value,tmp)!=0)
					{
						cell->SetText(tmp);
					}
				}
			}
			else if (col==7)
			{				
				mst_grid=OverViewOrderGrid::m_OverviewOrder_Grid_array[rows_no];
				CString tmp=mst_grid.PL ;
				if(!tmp.IsEmpty())
				{
					CString str_get_value=cell->GetText();
					if (wcscmp(str_get_value,tmp)!=0)
					{
						cell->SetText(tmp);
				
					}
				}
			}

			else if (col==8)
			{	
				
				mst_grid=OverViewOrderGrid::m_OverviewOrder_Grid_array[rows_no];
				CString tmp=mst_grid.Status;	
				if(!tmp.IsEmpty())
				{
					CString str_get_value=cell->GetText();
					if (wcscmp(str_get_value,tmp)!=0)
					{
						cell->SetText(tmp);
					}
				}
			}
			else if (col==9)
			{	
				
				mst_grid=OverViewOrderGrid::m_OverviewOrder_Grid_array[rows_no];
				CString tmp=mst_grid.Trade_Checked;	
				if(!tmp.IsEmpty())
				{
					CString str_get_value=cell->GetText();
					if (wcscmp(str_get_value,tmp)!=0)
					{
						cell->SetText(tmp);
					}
				}
			}
		
	   }

}
