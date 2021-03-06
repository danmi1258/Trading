#include "stdafx.h"
#include "resource.h"
#include "AnalysisGrid.h"
#include <afxdisp.h>
#include <time.h>
#include <locale.h>
#include "MainFrm.h"
#include "document.h"
#include <iostream>
#include "CNetPosTable.h"
#include "Login.h"
#include "clr_setting.h"
#include "TableGroupWiseNetPos.h"
using namespace std;
using namespace rapidjson;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//CUIThread
BEGIN_MESSAGE_MAP(GroupWise2NetPos,CUGCtrl)	
	ON_WM_SIZE()
	ON_WM_TIMER()		
END_MESSAGE_MAP()
int GroupWise2NetPos::insertFilterFlag=0;
GroupWise2NetPos::st_Dealing_Array GroupWise2NetPos::m_st_Dealing_Array;	
GroupWise2NetPos::st_Dealing_Array GroupWise2NetPos::m_st_Dealing_Grid_array;
GroupWise2NetPos::st_Dealing_Array GroupWise2NetPos::m_st_Dealing_Grid_udaptearray;
GroupWise2NetPos::st_Dealing_Array GroupWise2NetPos::m_st_Dealing_Grid_finalarray;
CMutex GroupWise2NetPos::dealing_mutex;
int GroupWise2NetPos::Data_Update=0;
/////////////////////////////////////////////////////////////////////////////
//Standard MyCug construction/destruction
boolean  Check_numeric_col_filter_3(CString  filter_value,CString  real_value);

CMutex gfliterLock;
CMutex gfinallock;
int gcol_click,g_a_d;
CString gcol0_val=L"";
CString gcol1_val=L"";
CString gcol2_val=L"";
CString gcol3_val=L"";
CString gcol4_val=L"";
CString gcol5_val=L"";
CString gcol6_val=L"";
CString gcol7_val=L"";
CString gcol8_val=L"";
UINT Show_Groupwise2NetPos(void *pParam);
UINT update_groupwise2NetPos(void *pParam);
GroupWise2NetPos::GroupWise2NetPos()
{
	UGXPThemes::UseThemes(false);	
	gcol_click=0;
	g_a_d=0;
}

GroupWise2NetPos::~GroupWise2NetPos()
{
	thread_destoy();
	UGXPThemes::CleanUp();
}

void GroupWise2NetPos::InitMenu()
{
	CMenu submenu;
	CMenu * menu = GetPopupMenu();

	EnableMenu(TRUE);

	menu->AppendMenuW(MF_STRING|MF_UNCHECKED,2001,_T("Filter"));


	submenu.CreatePopupMenu();
}
void GroupWise2NetPos::OnSetup()
{
	try
	{					
		int	nIndex = 0;
		CUGCell cell;
		GetGridDefault(&cell);
		SetGridDefault(&cell);
		EnableExcelBorders(FALSE);
		SetHighlightRow(TRUE, FALSE);
		SetDoubleBufferMode(TRUE);
		SetUserSizingMode( TRUE );			
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
		SetNumberCols( 7);		
		m_iArrowIndex = AddCellType( &m_sortArrow );
		SetCurrentCellMode( 2 );
		UseHints( TRUE );
		SetSH_Width( 0 );
		QuickSetCellType( 0, -1, m_iArrowIndex );
		SortBy(0, UG_SORT_ASCENDING );
		m_iSortCol = 0;
		m_bSortedAscending = TRUE;		
		InitMenu();

	}
	catch(_com_error & ce)
	{
		AfxMessageBox(ce.Description()+L"   writeData");			
	} 
}
void GroupWise2NetPos::data_ThreadStart()
{
	Data_Update=1;

	m_pThreads=AfxBeginThread(Show_Groupwise2NetPos, this);	
	m_pThreads=AfxBeginThread(update_groupwise2NetPos, this);
}
void GroupWise2NetPos::OnSheetSetup(int sheetNumber)
{
	int	nRow = 0, nCol = 0;	
	CUGCell cell;	
	SetGridDefault(&cell);
	EnableExcelBorders(TRUE);
	SetHighlightRow(TRUE, FALSE);
	SetDoubleBufferMode(TRUE);
	SetDefFont(0);
	SetSH_Width(0);		

	SetNumberCols(9);

	QuickSetText(0,-1,L"Group");
	SetColWidth(0,130);

	QuickSetText(1,-1,L"SYMBOL");
	SetColWidth(1,90);

	QuickSetText(2,-1,L"PreNetQty");	
	SetColWidth(2,75);

	QuickSetText(3,-1,L"DiffQty");	
	SetColWidth(3,75);

	QuickSetText(4,-1,L"NetQty");	
	SetColWidth(4,75);

	QuickSetText(5,-1,L"AVG RATE");
	SetColWidth(5,75);

	QuickSetText(6,-1,L"LASTRATE");
	SetColWidth(6,100);

	QuickSetText(7,-1,L"PL");
	SetColWidth(7,100);

	QuickSetText(8,-1,L"TOTAL BAL");	
	SetColWidth(8,150);

	// Header font
	for(int i = 0; i < GetNumberCols(); i++)
	{
		QuickSetFont(i, -1, 1);
	}	

	SetTimer(0, 100, NULL);


}

void GroupWise2NetPos::OnTimer(UINT nIDEvent)
{		

	//m_st_Dealing_Grid_array.Assign(m_st_Dealing_Array);	
	int r_count=GroupWise2NetPos::m_st_Dealing_Grid_udaptearray.Total();
	gfinallock.Lock();	
	int grid_total=GetNumberRows();

	if (GroupWise2NetPos::insertFilterFlag==1)
	{
		r_count=r_count+1;
	}		
	if (grid_total!=r_count)
	{			
		if (r_count>0)
		{
			SetNumberRows(r_count);					
		}
	}
	else
	{			
		RedrawAll();			
	}	
	gfinallock.Unlock();
}		


void GroupWise2NetPos::OnGetCell(int col,long row,CUGCell *cell)
{
	//m_logfile_g.LogEvent(L"Start OnGetCell");
	st_Dealing mst_grid={};
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
		if (GroupWise2NetPos::insertFilterFlag==1)
		{
			rows_no=row-1;				
			if (row==0)
			{
				return;
			}
		}
		if (col==0)
		{				
			mst_grid=m_st_Dealing_Grid_finalarray[rows_no];				
			CString tmp=mst_grid.Group ;				
			CString str_get_value=cell->GetText();
			if (wcscmp(str_get_value,tmp)!=0)
			{
				cell->SetText(tmp);
			}
		}
		else if (col==1)
		{

			mst_grid=m_st_Dealing_Grid_finalarray[rows_no];
			CString tmp=mst_grid.Symbol ;
			CString str_get_value=cell->GetText();
			if (wcscmp(str_get_value,tmp)!=0)
			{
				cell->SetText(tmp);
			}
		}
		else if (col==2)
		{				
			mst_grid=m_st_Dealing_Grid_finalarray[rows_no];
			double m_netqty=mst_grid.preNetqty  ;

			CString tmp=L"" ;
			tmp.Format(L"%.0f",m_netqty);
			CString str_get_value=cell->GetText();
			if (wcscmp(str_get_value,tmp)!=0)
			{
				cell->SetText(tmp);
			}
		}
		else if (col==3)
		{				
			mst_grid=m_st_Dealing_Grid_finalarray[rows_no];
			double  m_avgrate=mst_grid.Diffqty  ;
			CString tmp=L"" ;
			tmp.Format(L"%.2f",m_avgrate);
			CString str_get_value=cell->GetText();
			if (wcscmp(str_get_value,tmp)!=0)
			{
				cell->SetText(tmp);
			}
		}
		else if (col==4)
		{				
			mst_grid=m_st_Dealing_Grid_finalarray[rows_no];
			double m_netqty=mst_grid.Netqty ;

			CString tmp=L"" ;
			tmp.Format(L"%.0f",m_netqty);
			CString str_get_value=cell->GetText();
			if (wcscmp(str_get_value,tmp)!=0)
			{
				cell->SetText(tmp);
			}
		}
		else if (col==5)
		{				
			mst_grid=m_st_Dealing_Grid_finalarray[rows_no];
			double  m_avgrate=mst_grid.avg_rate ;
			CString tmp=L"" ;
			tmp.Format(L"%.4f",m_avgrate);
			CString str_get_value=cell->GetText();
			if (wcscmp(str_get_value,tmp)!=0)
			{
				cell->SetText(tmp);
			}
		}
		else if (col==6)
		{	
			mst_grid=m_st_Dealing_Grid_finalarray[rows_no];
			CString tmp=L"" ;
			tmp.Format(L"%.4f",mst_grid.current_rate);
			CString str_get_value=cell->GetText();
			if (wcscmp(str_get_value,tmp)!=0)
			{
				cell->SetText(tmp);
			}
		}
		else if (col==7)
		{				
			mst_grid=m_st_Dealing_Grid_finalarray[rows_no];
			CString tmp=L"";
			tmp.Format(L"%.4f",mst_grid.pl);
			CString str_get_value=cell->GetText();
			if (wcscmp(str_get_value,tmp)!=0)
			{
				cell->SetText(tmp);
			}
		}
		else if (col==8)
		{	

			mst_grid=m_st_Dealing_Grid_finalarray[rows_no];
			CString tmp=L"";
			tmp.Format(L"%.4f",mst_grid.Total_Amount);											
			CString str_get_value=cell->GetText();
			if (wcscmp(str_get_value,tmp)!=0)
			{
				cell->SetText(tmp);
			}
		}

	}					
}
void GroupWise2NetPos::OnMenuCommand(int col,long row,int section,int item)
{
	UNREFERENCED_PARAMETER(col);
	UNREFERENCED_PARAMETER(row);
	UNREFERENCED_PARAMETER(section);
	// init. local variables

	CMenu* pMenu;
	pMenu = GetPopupMenu();

	CMenu* submenu=pMenu->GetSubMenu(1);

	switch( item )
	{
	case 2001:
		{
			filter();
			break;
		}		  
	}
}
void GroupWise2NetPos::filter()
{
	try
	{
		CMenu *pMnenu;
		pMnenu= GetPopupMenu();

		if ( GroupWise2NetPos::insertFilterFlag==0)
		{
			addItemToCombobox();
			GroupWise2NetPos::insertFilterFlag=1;
			InsertRow(0);
			for (int col_count=0;col_count<9;col_count++)
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
			pMnenu->CheckMenuItem(2001,MF_CHECKED);

		}
		else
		{

			DeleteRow(0);
			GroupWise2NetPos::insertFilterFlag=0;
			gcol0_val=L"";
			gcol1_val=L"";
			gcol2_val=L"";
			gcol3_val=L"";
			gcol4_val=L"";
			gcol5_val=L"";
			gcol6_val=L"";
			gcol7_val=L"";
			gcol8_val=L"";

			pMnenu->CheckMenuItem(2001,MF_UNCHECKED);
		}
		RedrawAll();
	}
	catch(_com_error & ce)
	{
		AfxMessageBox(ce.Description()+L"   filter");			
	} 
}



void GroupWise2NetPos::addItemToCombobox()
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
	try
	{
		gfliterLock.Lock();
		int rows=1;
		rows=m_st_Dealing_Grid_finalarray.Total();	
		CString str_val=L"";	
		for (int forcount=0;forcount<9;forcount++)
		{
			str[forcount]=L"ALL\n";		
		}
		for (int forcount=0;forcount<rows;forcount++)
		{
			st_Dealing m_st_Netposition={};
			m_st_Netposition=m_st_Dealing_Array[forcount];
			for (int clocount=0;clocount<9;clocount++)
			{											
				if (clocount==0)
				{

					str_val=m_st_Netposition.Group;
					str_val=str_val.Trim();
					if (CheckvalueInArray(arr,str_val)==false && str_val!=L"")
					{
						str[0]=str[0]+str_val+L"\n";										
						arr.Add(str_val);							
					}
				}
				if (clocount==1)
				{
					str_val=m_st_Netposition.Symbol ;
					str_val=str_val.Trim();
					if (CheckvalueInArray(arr1,str_val)==false  && str_val!=L"")
					{
						str[1]=str[1]+str_val+L"\n";										
						arr1.Add(str_val);
					}
				}


				if (clocount==2)
				{
					double  u_deal=m_st_Netposition.preNetqty  ;
					str_val.Format(L"%.0f",m_st_Netposition.preNetqty  );
					str_val=str_val.Trim();
					if (CheckvalueInArray(arr2,str_val)==false  && str_val!=L"")
					{
						str[2]=str[2]+str_val+L"\n";										
						arr2.Add(str_val);
					}
				}


				if (clocount==3)
				{
					double  u_deal=m_st_Netposition.Diffqty ;
					str_val.Format(L"%.0f",m_st_Netposition.Diffqty);
					str_val=str_val.Trim();
					if (CheckvalueInArray(arr3,str_val)==false  && str_val!=L"")
					{
						str[3]=str[3]+str_val+L"\n";										
						arr3.Add(str_val);
					}
				}

				if (clocount==4)
				{
					double  u_deal=m_st_Netposition.Netqty ;
					str_val.Format(L"%.0f",m_st_Netposition.Netqty );
					str_val=str_val.Trim();
					if (CheckvalueInArray(arr4,str_val)==false  && str_val!=L"")
					{
						str[4]=str[4]+str_val+L"\n";										
						arr4.Add(str_val);
					}
				}
				if (clocount==5)
				{
					double  u_order=m_st_Netposition.avg_rate ;
					str_val.Format(L"%.4f",u_order) ;
					str_val=str_val.Trim();
					if (CheckvalueInArray(arr5,str_val)==false  && str_val!=L"")
					{
						str[5]=str[5]+str_val+L"\n";										
						arr5.Add(str_val);
					}
				}

				if (clocount==6)
				{
					double current_rate=m_st_Netposition.current_rate  ;
					str_val.Format(L"%.4f",current_rate);
					if (CheckvalueInArray(arr6,str_val)==false  && str_val!=L"")
					{
						str[6]=str[6]+str_val+L"\n";										
						arr6.Add(str_val);
					}
				}
				if (clocount==7)
				{

					str_val.Format(L"%.4f",m_st_Netposition.pl);
					if (CheckvalueInArray(arr7,str_val)==false  && str_val!=L"")
					{
						str[7]=str[7]+str_val+L"\n";										
						arr7.Add(str_val);
					}
				}
				if (clocount==8)
				{						
					str_val.Format(L"%.4f",m_st_Netposition.Total_Amount);
					if (CheckvalueInArray(arr8,str_val)==false  && str_val!=L"")
					{
						str[8]=str[8]+str_val+L"\n";										
						arr8.Add(str_val);
					}
				}					

			}	
		}
		for(int f=0;f<9;f++)
		{
			colShorting(f);
		}	
		gfliterLock.Unlock();
	}
	catch(_com_error & ce)
	{
		AfxMessageBox(ce.Description()+L"addItemToCombobox");			
	} 
}

BOOLEAN  GroupWise2NetPos::CheckvalueInArray(const CStringArray& arr,CString strval) 
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

void GroupWise2NetPos::colShorting( int index)
{
	m_str_array.Clear();
	CString cstr=str[index];
	int str_len=cstr.GetLength();
	CString split_str=L"";
	for (int f=0;f<str_len;f++)
	{		
		if (cstr.Mid(f,1)!=L"\n")
		{
			split_str=split_str+cstr.Mid(f,1);
		}
		else
		{
			if (split_str.Trim().GetLength()>0 && split_str.Trim()!="ALL")
			{
				st_shorting m_st_shorting={};
				CMTStr::Copy(m_st_shorting.m_symbol,split_str);
				m_str_array.Add(&m_st_shorting);
			}
			split_str=L"";
		}
	}	
	int arr_count=m_str_array.Total();
	st_shorting  first_st={};
	st_shorting  next_st={};
	st_shorting  swap_st={};
	for (int f=0;f<arr_count;f++)
	{		
		first_st=m_str_array[f] ;
		for (int j=f+1;j<arr_count;j++)
		{			
			next_st=m_str_array[j];			
			if (wcscmp(first_st.m_symbol,next_st.m_symbol)>0)
			{
				m_str_array.Shift(j,f-j);
				swap_st=m_str_array[f];
				first_st=swap_st;							
			}
		}
	}
	st_shorting st_str={};
	CString final_str=L"";
	for (int f=0;f<arr_count+1;f++)
	{
		st_str=m_str_array[f];
		CString temp_str=st_str.m_symbol ;
		final_str=final_str+L"\n"+temp_str;
	}
	str[index]=L"ALL"+final_str;
}



boolean  Check_numeric_col_filter_3(CString  filter_value,CString  real_value)
{
	boolean bool_col=false;		
	double temp_double=0;
	LPTSTR endPtr;
	temp_double= _tcstod(real_value, &endPtr);	
	filter_value=filter_value.Trim();
	if (filter_value.GetLength()>2)
	{
		if (filter_value.Mid(0,2)==L">=")
		{
			CString o_real_val=filter_value.Mid(2,filter_value.GetLength()-2);
			double db_filter_val=0;
			LPTSTR endPtr1;				
			db_filter_val= _tcstod(o_real_val, &endPtr1);	
			if (temp_double>=db_filter_val)
			{
				bool_col=true;
				return bool_col;
			}
		}
		if (filter_value.Mid(0,2)==L"<=")
		{
			CString o_real_val=filter_value.Mid(2,filter_value.GetLength()-2);
			double db_filter_val=0;
			LPTSTR endPtr1;				
			db_filter_val= _tcstod(o_real_val, &endPtr1);	
			if (temp_double<=db_filter_val)
			{
				bool_col=true;
				return bool_col;
			}
		}

		if (filter_value.Mid(0,1)==L"<")
		{
			CString o_real_val=filter_value.Mid(1,filter_value.GetLength()-1);
			double db_filter_val=0;
			LPTSTR endPtr1;				
			db_filter_val= _tcstod(o_real_val, &endPtr1);	
			if (temp_double<db_filter_val)
			{
				bool_col=true;
				return bool_col;
			}
		}
		if (filter_value.Mid(0,1)==L">")
		{
			CString o_real_val=filter_value.Mid(1,filter_value.GetLength()-1);
			double db_filter_val=0;
			LPTSTR endPtr1;				
			db_filter_val= _tcstod(o_real_val, &endPtr1);	
			if (temp_double>db_filter_val)
			{
				bool_col=true;
				return bool_col;
			}
		}	


		if (filter_value.Mid(0,1)!=L">" &&  filter_value.Mid(0,1)!=L"<")
		{
			double db_filter_val=0;
			LPTSTR endPtr1;				
			db_filter_val= _tcstod(filter_value, &endPtr1);	
			if (temp_double==db_filter_val)
			{
				bool_col=true;
				return bool_col;
			}
		}

	}
	else
	{

		if (filter_value.Mid(0,1)==L"<")
		{
			CString o_real_val=filter_value.Mid(1,filter_value.GetLength()-1);
			double db_filter_val=0;
			LPTSTR endPtr1;				
			db_filter_val= _tcstod(o_real_val, &endPtr1);	
			if (temp_double<db_filter_val)
			{
				bool_col=true;
				return bool_col;
			}
		}
		if (filter_value.Mid(0,1)==L">")
		{
			CString o_real_val=filter_value.Mid(1,filter_value.GetLength()-1);
			double db_filter_val=0;
			LPTSTR endPtr1;				
			db_filter_val= _tcstod(o_real_val, &endPtr1);	
			if (temp_double>db_filter_val)
			{
				bool_col=true;
				return bool_col;
			}
		}	

		if (filter_value.Mid(0,1)!=L">" &&  filter_value.Mid(0,1)!=L"<")
		{
			double db_filter_val=0;
			LPTSTR endPtr1;				
			db_filter_val= _tcstod(filter_value, &endPtr1);	
			if (temp_double==db_filter_val)
			{
				bool_col=true;
				return bool_col;
			}
		}
	}
	return bool_col;
}
int GroupWise2NetPos::OnCellTypeNotify(long ID,int col,long row,long msg,long param)
{
	if(ID == UGCT_DROPLIST)
	{
		return OnDropList(ID, col, row, msg, param);
	}
	return 0;
}
int GroupWise2NetPos::OnDropList(long ID,int col,long row,long msg,long param)
{	
	if (msg==103)
	{		
		if(GroupWise2NetPos::insertFilterFlag==1 && row==0 )
		{
			CString  strval=L"";
			CUGCell cell;
			GetCell(col,row,&cell);
			strval=cell.GetText();	
			if(col==0)
			{
				if (strval!=L"")
				{
					gcol0_val=strval;					
				}
				else
				{
					gcol0_val=L"ALL";					
				}
			}


			if(col==1)
			{
				if (strval!=L"")
				{
					gcol1_val=strval;					
				}
				else
				{
					gcol1_val=L"ALL";					
				}
			}

			if(col==2)
			{
				if (strval!=L"")
				{
					gcol2_val=strval;					
				}
				else
				{
					gcol2_val=L"ALL";					
				}
			}

			if(col==3)
			{
				if (strval!=L"")
				{
					gcol3_val=strval;					
				}
				else
				{
					gcol3_val=L"ALL";					
				}
			}

			if(col==4)
			{
				if (strval!=L"")
				{
					gcol4_val=strval;					
				}
				else
				{
					gcol4_val=L"ALL";					
				}
			}

			if(col==5)
			{
				if (strval!=L"")
				{
					gcol5_val=strval;					
				}
				else
				{
					gcol5_val=L"ALL";					
				}
			}

			if(col==6)
			{
				if (strval!=L"")
				{
					gcol6_val=strval;					
				}
				else
				{
					gcol6_val=L"ALL";					
				}
			}

			if(col==7)
			{
				if (strval!=L"")
				{
					gcol7_val=strval;					
				}
				else
				{
					gcol7_val=L"ALL";					
				}
			}

			if(col==8)
			{
				if (strval!=L"")
				{
					gcol8_val=strval;					
				}
				else
				{
					gcol8_val=L"ALL";					
				}
			}
		}		
	}	
	return true;
}

UINT Show_Groupwise2NetPos(void *pParam)
{
	GroupWise2NetPos* pThis= (GroupWise2NetPos*)pParam;	
	CoInitialize(NULL);
	CDataSource connection;
	CSession session;
	CCommand<CAccessor<CGroupWiseNetPos> > artists1;	
	HRESULT hr;
	hr=connection.OpenFromInitializationString(L"Provider=SQLNCLI11.1;Password=ok@12345;Persist Security Info=False;User ID=sa;Initial Catalog=CHECKDATA;Data Source=64.251.7.161;Use Procedure for Prepare=1;Auto Translate=True;Packet Size=4096;Workstation ID=WINDOWS-LOJSHQK;Initial File Name=\"\";Use Encryption for Data=False;Tag with column collation when possible=False;MARS Connection=False;DataTypeCompatibility=0;Trust Server Certificate=False;Application Intent=READWRITE");
	if(SUCCEEDED(hr))
	{
		hr=session.Open(connection);
		while (GroupWise2NetPos::Data_Update==1)
		{	
			Sleep(100);
			CString strCommand=L"";		
			strCommand.Format(L"GroupWise2NetPos'%d'",CMainFrame::int_currenttime);        
			_bstr_t bstrCommand="";
			bstrCommand=strCommand;
			char* strCommand_char=(char*)bstrCommand;
			if(SUCCEEDED(hr))
			{
				hr=artists1.Open(session,strCommand_char);							 
			}
			GroupWise2NetPos::dealing_mutex.Lock();			
			if(SUCCEEDED(hr))
			{
				GroupWise2NetPos::m_st_Dealing_Array.Clear();
				

				while (hr=artists1.MoveNext() == S_OK)
				{						
					GroupWise2NetPos::st_Dealing m_st_Dealing={};
					CMTStr::Copy(m_st_Dealing.Group ,artists1.Group );				 					
					CMTStr::Copy(m_st_Dealing.Symbol ,artists1.Symbol );				 										

					m_st_Dealing.preNetqty =artists1.PreNetqty  ;
					m_st_Dealing.Diffqty =artists1.Diffqty  ;

					m_st_Dealing.Netqty=artists1.Netqty ;
					m_st_Dealing.avg_rate =artists1.avg_rate  ;
					m_st_Dealing.current_rate  =artists1.current_rate ;
					m_st_Dealing.pl =artists1.pl ;
					m_st_Dealing.Total_Amount=artists1.Total_Amount;

					GroupWise2NetPos::m_st_Dealing_Array.Add(&m_st_Dealing);
				}
				artists1.Close();				    									 			 				 
			}
			GroupWise2NetPos::dealing_mutex.Unlock();


			gfliterLock.Lock();
			GroupWise2NetPos::m_st_Dealing_Grid_array.Assign(GroupWise2NetPos::m_st_Dealing_Array);
			gfliterLock.Unlock();	

		}
		session.Close();
		connection.Close();
	}
	return 0;
}

UINT update_groupwise2NetPos(void *pParam)
{

	while(GroupWise2NetPos::Data_Update==1)
	{
		gfliterLock.Lock();
		int noof_rowsInStruc=GroupWise2NetPos::m_st_Dealing_Grid_array.Total();	
		if (noof_rowsInStruc>0)
		{
			GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Clear();
		}
		CString getColvalue=L"";
		for(int fcount=0;fcount<noof_rowsInStruc;fcount++)
		{
			if (GroupWise2NetPos::insertFilterFlag==1)
			{
				GroupWise2NetPos::st_Dealing m_st_Netposition={};
				m_st_Netposition=GroupWise2NetPos::m_st_Dealing_Grid_array[fcount];

				int flag=0;				
				CString col_row_val[9];
				CString str_temp=L"";

				col_row_val[0]=m_st_Netposition.Group  ;
				if (gcol0_val.Trim().GetLength()>0)
				{
					col_row_val[0]=col_row_val[0].Mid(0,gcol0_val.Trim().GetLength());
				}
				col_row_val[1]=m_st_Netposition.Symbol   ;
				if (gcol1_val.Trim().GetLength()>0)
				{
					col_row_val[1]=col_row_val[1].Mid(0,gcol1_val.Trim().GetLength());
				}


				
				str_temp.Format(L"%.0f",m_st_Netposition.preNetqty  );
				col_row_val[2]=str_temp  ;
				if (gcol2_val.Trim().GetLength()>0)
				{
					col_row_val[2]=col_row_val[2].Mid(0,gcol2_val.Trim().GetLength());
				}

				str_temp.Format(L"%.2f",m_st_Netposition.Diffqty  );
				col_row_val[3]=str_temp  ;		
				if (gcol3_val.Trim().GetLength())
				{
					col_row_val[3]=col_row_val[3].Mid(0,gcol3_val.Trim().GetLength());
				}

				str_temp.Format(L"%.0f",m_st_Netposition.Netqty );
				col_row_val[4]=str_temp  ;
				if (gcol4_val.Trim().GetLength()>0)
				{
					col_row_val[4]=col_row_val[4].Mid(0,gcol4_val.Trim().GetLength());
				}

				str_temp.Format(L"%.4f",m_st_Netposition.avg_rate );
				col_row_val[5]=str_temp  ;		
				if (gcol5_val.Trim().GetLength()>0)
				{
					col_row_val[5]=col_row_val[5].Mid(0,gcol5_val.Trim().GetLength());
				}

				str_temp.Format(L"%.4f",m_st_Netposition.current_rate );
				col_row_val[6]=str_temp ;
				if (gcol6_val.Trim().GetLength()>0)
				{
					col_row_val[6]=col_row_val[6].Mid(0,gcol6_val.Trim().GetLength());
				}

				str_temp.Format(L"%.4f",m_st_Netposition.pl  );
				col_row_val[7]=str_temp  ;
				if (gcol7_val.Trim().GetLength()>0)
				{
					col_row_val[7]=col_row_val[7].Mid(0,gcol7_val.Trim().GetLength());
				}

				str_temp.Format(L"%.4f",m_st_Netposition.Total_Amount);
				col_row_val[8]=str_temp;
				if (gcol8_val.Trim().GetLength()>0)
				{
					col_row_val[8]=col_row_val[8].Mid(0,gcol8_val.Trim().GetLength());
				}			

				if((gcol0_val.Trim()==col_row_val[0].Trim() || gcol0_val.Trim()==L"ALL"||gcol0_val.Trim()==L"") && (gcol1_val.Trim()==col_row_val[1].Trim() || gcol1_val.Trim()==L"ALL"||gcol1_val.Trim()==L"") && (gcol2_val.Trim()==col_row_val[2].Trim() || gcol2_val.Trim()==L"ALL"||gcol2_val.Trim()==L"")  && (gcol3_val.Trim()==col_row_val[3].Trim() || gcol3_val.Trim()==L"ALL"||gcol3_val.Trim()==L"")  && (gcol4_val.Trim()==col_row_val[4].Trim() || gcol4_val.Trim()==L"ALL"||gcol4_val.Trim()==L"")   && (gcol5_val.Trim()==col_row_val[5].Trim() ||gcol5_val.Trim()==L"ALL"||gcol5_val.Trim()==L"")   && (gcol6_val.Trim()==col_row_val[6].Trim() || gcol6_val.Trim()==L"ALL"||gcol6_val.Trim()==L"")   && (gcol7_val.Trim()==col_row_val[7].Trim() || gcol7_val.Trim()==L"ALL"||gcol7_val.Trim()==L"")  && (gcol8_val.Trim()==col_row_val[8].Trim() || gcol8_val.Trim()==L"ALL"||gcol8_val.Trim()==L""))
				{						
					GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Add(&m_st_Netposition);						
				}

			}
			else
			{
				GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Assign(GroupWise2NetPos::m_st_Dealing_Grid_array);	

			}
		}
		gfliterLock.Unlock();

		//Shorting Column
		int t_rows=GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Total();
		GroupWise2NetPos::st_Dealing first_st={};
		GroupWise2NetPos::st_Dealing swap_st={};
		GroupWise2NetPos::st_Dealing next_st={};


		int val_type=0;
		if (gcol_click==0||gcol_click==1)
		{
			val_type=0;
		}
		else
		{
			val_type=1;
		}
		for (int i=0;i<t_rows;i++)
		{
			first_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
			for (int j=i+1;j<t_rows;j++)
			{
				next_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[j];
				if (g_a_d==0)
				{
					if (val_type==0)
					{
						if (gcol_click==0)
						{
							if (wcscmp(first_st.Group ,next_st.Group )>0)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								CMTStr::Copy(first_st.Group ,swap_st.Group );							
							}
						}
						if (gcol_click==1)
						{
							if (wcscmp(first_st.Symbol  ,next_st.Symbol  )>0)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								CMTStr::Copy(first_st.Symbol  ,swap_st.Symbol );							
							}
						}
					}
					else
					{
						if (gcol_click==2)
						{															
							double d_val1=first_st.preNetqty ;															
							double d_val2=next_st.preNetqty;						
							if (d_val1>d_val2)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								first_st.preNetqty =swap_st.preNetqty ;							
							}
						}

						if (gcol_click==3)
						{															
							double d_val1=first_st.Diffqty  ;															
							double d_val2=next_st.Diffqty;						
							if (d_val1>d_val2)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								first_st.Diffqty =swap_st.Diffqty ;							
							}
						}
						if (gcol_click==4)
						{															
							double d_val1=first_st.Netqty   ;															
							double d_val2=next_st.Netqty;						
							if (d_val1>d_val2)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								first_st.Netqty =swap_st.Netqty ;							
							}
						}
						if (gcol_click==5)
						{															
							double d_val1=first_st.avg_rate    ;															
							double d_val2=next_st.avg_rate;						
							if (d_val1>d_val2)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								first_st.avg_rate =swap_st.avg_rate ;							
							}
						}
						if (gcol_click==6)
						{															
							double d_val1=first_st.current_rate    ;															
							double d_val2=next_st.current_rate;						
							if (d_val1>d_val2)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								first_st.current_rate =swap_st.current_rate ;							
							}
						}
						if (gcol_click==7)
						{															
							double d_val1=first_st.pl  ;															
							double d_val2=next_st.pl;
							if (d_val1>d_val2)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								first_st.pl =swap_st.pl;							
							}
						}
						if (gcol_click==8)
						{															
							double d_val1=first_st.Total_Amount   ;															
							double d_val2=next_st.Total_Amount;
							if (d_val1>d_val2)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								first_st.Total_Amount =swap_st.Total_Amount;					
							}
						}

					}
				}
				else
				{
					if (val_type==0)
					{
						if (gcol_click==0)
						{
							if (wcscmp(first_st.Group ,next_st.Group )<0)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								CMTStr::Copy(first_st.Group ,swap_st.Group );							
							}
						}
						if (gcol_click==1)
						{
							if (wcscmp(first_st.Symbol  ,next_st.Symbol  )<0)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								CMTStr::Copy(first_st.Symbol  ,swap_st.Symbol );							
							}
						}
					}
					else
					{
						if (gcol_click==2)
						{															
							double d_val1=first_st.preNetqty ;															
							double d_val2=next_st.preNetqty;						
							if (d_val1<d_val2)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								first_st.preNetqty =swap_st.preNetqty ;							
							}
						}

						if (gcol_click==3)
						{															
							double d_val1=first_st.Diffqty  ;															
							double d_val2=next_st.Diffqty;						
							if (d_val1<d_val2)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								first_st.Diffqty =swap_st.Diffqty ;							
							}
						}
						if (gcol_click==4)
						{															
							double d_val1=first_st.Netqty   ;															
							double d_val2=next_st.Netqty;						
							if (d_val1<d_val2)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								first_st.Netqty =swap_st.Netqty ;							
							}
						}
						if (gcol_click==5)
						{															
							double d_val1=first_st.avg_rate    ;															
							double d_val2=next_st.avg_rate;						
							if (d_val1<d_val2)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								first_st.avg_rate =swap_st.avg_rate ;							
							}
						}
						if (gcol_click==6)
						{															
							double d_val1=first_st.current_rate    ;															
							double d_val2=next_st.current_rate;						
							if (d_val1<d_val2)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								first_st.current_rate =swap_st.current_rate ;							
							}
						}
						if (gcol_click==7)
						{															
							double d_val1=first_st.pl  ;															
							double d_val2=next_st.pl;
							if (d_val1<d_val2)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								first_st.pl =swap_st.pl;							
							}
						}
						if (gcol_click==8)
						{															
							double d_val1=first_st.Total_Amount   ;															
							double d_val2=next_st.Total_Amount;
							if (d_val1<d_val2)
							{
								GroupWise2NetPos::m_st_Dealing_Grid_finalarray.Shift(j,i-j);
								swap_st=GroupWise2NetPos::m_st_Dealing_Grid_finalarray[i];
								first_st.Total_Amount =swap_st.Total_Amount;					
							}
						}

					}
				}
			}
		}

		gfinallock.Lock();
		GroupWise2NetPos::m_st_Dealing_Grid_udaptearray.Assign(GroupWise2NetPos::m_st_Dealing_Grid_finalarray);
		gfinallock.Unlock();
	}
	return 0;
	//End of Shorting Col
}


void GroupWise2NetPos::OnTH_LClicked(int col,long row,int updn,RECT *rect,POINT *point,BOOL processed)
{		
	UNREFERENCED_PARAMETER(row);
	UNREFERENCED_PARAMETER(rect);
	UNREFERENCED_PARAMETER(point);
	UNREFERENCED_PARAMETER(processed);

	gcol_click=col;
	if( updn == 0)
		return;
	if (gcol_click!=col)
	{
		g_a_d=0;
	}
	else
	{
		if (g_a_d==0)
		{
			g_a_d=1;
		}
		else
		{
			g_a_d=0;
		}
	}


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


void GroupWise2NetPos::thread_destoy()
{
	try 
	{		
		Data_Update=0;
		DWORD exit_code= NULL;
		if (COutputWnd::m_AnalysisGrid.m_pThreads != NULL)
		{
			if(WaitForSingleObject(COutputWnd::m_AnalysisGrid.m_pThreads->m_hThread,INFINITE) == WAIT_OBJECT_0) 
			{
				GetExitCodeThread(COutputWnd::m_AnalysisGrid.m_pThreads->m_hThread, &exit_code);
				if(exit_code == STILL_ACTIVE)
				{
					::TerminateThread(COutputWnd::m_AnalysisGrid.m_pThreads->m_hThread, 0);
					CloseHandle(COutputWnd::m_AnalysisGrid.m_pThreads->m_hThread);
				}
				//COutputWnd::m_AnalysisGrid.m_pThreads->m_hThread = NULL;
				COutputWnd::m_AnalysisGrid.m_pThreads = NULL;
			}
		}
	}
	catch(_com_error & ce)
	{
		AfxMessageBox(ce.Description()+L"Thread UnInitiliaze");			
	}
}