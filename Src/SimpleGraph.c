#include "DIALOG.h"

/*********************************************************************
 *
 *       Externals
 *
 **********************************************************************
 */

/*********************************************************************
 *
 *       Defines
 *
 **********************************************************************
 */

#include "arm_math.h"
#include "arm_const_structs.h"
#include "697hz.inc"
#include "main.h"

#define TEST_LENGTH_SAMPLES 512
#define FFT_SIZE 256
/* -------------------------------------------------------------------
 * External Input and Output buffer Declarations for FFT Bin Example
 * ------------------------------------------------------------------- */
float32_t FFT_Input[TEST_LENGTH_SAMPLES];
float32_t FFT_Output[TEST_LENGTH_SAMPLES / 2];

/* ------------------------------------------------------------------
 * Global variables for FFT Bin Example
 * ------------------------------------------------------------------- */
uint32_t fftSize = 1024;
uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;

/* Reference index at which max energy of bin ocuurs */
uint32_t refIndex = 213, testIndex = 0;

#define ID_WINDOW_0     (GUI_ID_USER + 0x00)
#define ID_GRAPH_0      (GUI_ID_USER + 0x01)

#define MESSAGE_STARTSTOP  (WM_USER + 0x00)

static GUI_POINT Points[DMA_BUFFER_LENGTH];
static GRAPH_DATA_Handle SineData;
WM_HWIN hItem;
static int Stop;
extern int AdcValue;
extern U16 dmaBuffer[DMA_BUFFER_LENGTH];
double Factor;
int tmpdata;
arm_status status;
float32_t maxValue;
/*********************************************************************
 *
 *       Static data
 *
 **********************************************************************
 */
/*********************************************************************
 *
 *       _aDialogCreate
 */
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
		{ WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 0, 0, 480, 272, 0, 0x0,0 },
		{ GRAPH_CreateIndirect, "Graph", ID_GRAPH_0, 1, 1, 478,270, 0, 0x0, 0 }};

/*********************************************************************
 *
 *       Static code
 *
 **********************************************************************
 */
/*********************************************************************
 *
 *       _cbGraph
 */
static void _cbGraph(WM_MESSAGE * pMsg) {
	GUI_PID_STATE * pState;
	static int Pressed;
	WM_HWIN hWin;

	switch (pMsg->MsgId) {
	case WM_TOUCH:
		pState = (GUI_PID_STATE *) pMsg->Data.p;
		if (pState) {
			if (pState->Pressed) {
				Pressed = 1;
			} else {
				if (Pressed) {
					Pressed = 0;
					//
					// If released send start stop message to the parent
					//
					hWin = WM_GetParent(pMsg->hWin);
					WM_SendMessageNoPara(hWin, MESSAGE_STARTSTOP);
				}
			}
		}
		break;
	default:
		GRAPH_Callback(pMsg);
		break;
	}
}

/*********************************************************************
 *
 *       _cbDialog
 */
static void _cbDialog(WM_MESSAGE * pMsg) {
	WM_HWIN hItem;
	//int NumItems;

//  static WM_HTIMER hTimer;

	switch (pMsg->MsgId) {
	case WM_INIT_DIALOG:
		//
		// Initialization of 'Graph'
		//
		hItem = WM_GetDialogItem(pMsg->hWin, ID_GRAPH_0);
		//
		// Set a callback, it manages touch on the graph, if it gets touched it stops, another touch and it starts
		//
		WM_SetCallback(hItem, _cbGraph);
		//
		// Make the grid visible
		//
		GRAPH_SetGridVis(hItem, 1);
		//
		// Get x size, used as max num items
		//
		//NumItems = WM_GetWindowSizeX(hItem);
		//
		// Create two data items, one for sin, one for cos
		//
		SineData = GRAPH_DATA_XY_Create(GUI_RED, 470, Points,
				GUI_COUNTOF(Points));
		//PhaseDataB = GRAPH_DATA_YT_Create(GUI_WHITE, NumItems, NULL, 0);
		//PhaseDataC = GRAPH_DATA_YT_Create(GUI_LIGHTCYAN, NumItems, NULL, 0);
		//
		// Attach them to the GRAPH
		//
		GRAPH_AttachData(hItem, SineData);
		//GRAPH_AttachData(hItem, PhaseDataB);
		//GRAPH_AttachData(hItem, PhaseDataC);
		//
		// Create a timer which updates the GRAPH
		//
		//hTimer = WM_CreateTimer(pMsg->hWin, 0, 10, 0);
		break;
		/*  case WM_TIMER:
		 //
		 // Depending on Stop, restart the graph
		 //
		 if (Stop == 0) {
		 WM_RestartTimer(hTimer, 10);
		 }
		 //
		 // Calculate new values for the graph and add them to the data items, the GRAPH gets updated automatically
		 //
		 NewPhaseDataA =GUI_sin(AngleA)/10;
		 NewPhaseDataB =GUI_sin(AngleB)/10;
		 NewPhaseDataC =GUI_sin(AngleC)/10;
		 AngleA+=10;
		 AngleB+=10;
		 AngleC+=10;
		 if(AngleA==0xfff){
		 AngleA=0;
		 AngleB=0x555;
		 AngleC=0xAAA;
		 }
		 GRAPH_DATA_YT_AddValue(PhaseDataA, NewPhaseDataA+131);
		 GRAPH_DATA_YT_AddValue(PhaseDataB, NewPhaseDataB+131);
		 GRAPH_DATA_YT_AddValue(PhaseDataC, NewPhaseDataC+131);
		 break;*/
	case MESSAGE_STARTSTOP:
		//
		// Start stop message send by the GRAPH
		//
		Stop ^= 1;
		/*    if (Stop == 0) {
		 WM_RestartTimer(hTimer, 10);
		 }*/
		break;
	default:
		WM_DefaultProc(pMsg);
		break;
	}
}

/*********************************************************************
 *
 *       _cbBk
 */
static void _cbBk(WM_MESSAGE * pMsg) {
	switch (pMsg->MsgId) {
	case WM_PAINT:
		GUI_SetBkColor(GUI_BLACK);
		GUI_Clear();
		break;
	default:
		WM_DefaultProc(pMsg);
		break;
	}
}
/*********************************************************************
 *
 *       Public code
 *
 **********************************************************************
 */
static void _UserDraw(WM_HWIN hWin, int Stage) {
	switch (Stage) {
	case GRAPH_DRAW_FIRST:
		break;
	case GRAPH_DRAW_LAST:
		GUI_SetColor(GUI_GREEN);
		for (int i = 1; i < FFT_SIZE; i++) {
			//GUI_DrawLine(Points[i].x, 262, Points[i].x, 262 - Points[i].y);
			GUI_DrawRect(2*(Points[i].x), 270 - Points[i].y, 2*(Points[i].x)+1, 270);
			//GUI_DrawVLine(Points[i].x,270 - Points[i].y, 270);
		}
		break;
	}
}
/*********************************************************************
 *
 *       MainTask
 */
void MainTask(void) {
	WM_HWIN hWin;
	WM_HWIN hItem;
	WM_SetCallback(WM_HBKWIN, _cbBk);
	hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate),_cbDialog, WM_HBKWIN, 0, 0);
	hItem = WM_GetDialogItem(hWin, ID_GRAPH_0);
	GRAPH_SetUserDraw(hItem, _UserDraw); // Enable custom draw function
	WM_InvalidateWindow(hWin);
}
void NewData(void) {
	if (Stop) {
	} else {

		Factor = AdcValue * 0.0000244140625;
		for (int i = 0; i < 470; i++) {
			Points[i].x = i;
			Points[i].y = (GUI_sin(8.712765957446809 * i) * Factor) + 131;
		}
		hItem = WM_GetFirstChild(WM_HBKWIN);
		hItem = WM_GetDialogItem(hItem, ID_GRAPH_0);
		GRAPH_DetachData(hItem, SineData);
		GRAPH_DATA_XY_Delete(SineData);
		SineData = GRAPH_DATA_XY_Create(GUI_GREEN, 470, Points,GUI_COUNTOF(Points));
		GRAPH_AttachData(hItem, SineData);
	}
}
void SAI_Data(void) {
	for (int i = 0; i < 235; i++) {
		Points[2 * i].x = 2 * i;
		Points[2 * i + 1].x = 2 * i + 1;
		dmaBuffer[2 * i] += 35536;
		dmaBuffer[2 * i] /= 256;
		Points[2 * i].y = dmaBuffer[2 * i];
		Points[2 * i + 1].y = dmaBuffer[2 * i];
	}
	hItem = WM_GetFirstChild(WM_HBKWIN);
	hItem = WM_GetDialogItem(hItem, ID_GRAPH_0);
	GRAPH_DetachData(hItem, SineData);
	GRAPH_DATA_XY_Delete(SineData);
	SineData = GRAPH_DATA_XY_Create(GUI_GREEN, GUI_COUNTOF(Points), Points,GUI_COUNTOF(Points));
	//GRAPH_DATA_XY_SetPenSize(SineData, 2);
	//GRAPH_DATA_XY_SetLineVis(SineData, 0);
	//GRAPH_DATA_XY_SetPointVis(SineData, 1);
	GRAPH_AttachData(hItem, SineData);
}
void FFT_Data(void) {
	static arm_rfft_instance_q15 fft_instance;
	static q15_t output[FFT_SIZE * 2]; //has to be twice FFT size
	if (arm_rfft_init_q15(&fft_instance, 256, 0, 1) != ARM_MATH_SUCCESS) {
		//error
	}
	arm_rfft_q15(&fft_instance, (q15_t*) __697hz_raw, output);
	arm_abs_q15(output, output, FFT_SIZE);

	for (int i = 0; i < FFT_SIZE; i++) {
		Points[i].x = i;
		Points[i].y = output[i] / 48;
	}
}
void FFT_Data2(void) {
	static arm_rfft_instance_q15 fft_instance;
	static q15_t output[FFT_SIZE * 2]; //has to be twice FFT size
	unsigned char input[DMA_BUFFER_LENGTH/2];
	for (int i = 0; i < DMA_BUFFER_LENGTH/2; i++) {
		dmaBuffer[2*i] += 35536;
		dmaBuffer[2*i]/=256;
		input[i]=dmaBuffer[2*i];
	}
	if (arm_rfft_init_q15(&fft_instance, 256, 0, 1) != ARM_MATH_SUCCESS) {
		//error
	}
	arm_rfft_q15(&fft_instance, (q15_t*) input, output);
	arm_abs_q15(output, output, FFT_SIZE);

	for (int i = 0; i < FFT_SIZE; i++) {
		Points[i].x = i;
		Points[i].y = output[i] / 4;
	}
	hItem = WM_GetFirstChild(WM_HBKWIN);
	hItem = WM_GetDialogItem(hItem, ID_GRAPH_0);
	WM_InvalidateWindow(hItem);
}
void RAW_Data(void){
	for (int i = 0; i < 470; i++) {
		Points[i].x = i;
		Points[i].y = __697hz_raw[i];
	}
	hItem = WM_GetFirstChild(WM_HBKWIN);
	hItem = WM_GetDialogItem(hItem, ID_GRAPH_0);
	GRAPH_DetachData(hItem, SineData);
	GRAPH_DATA_XY_Delete(SineData);
	SineData = GRAPH_DATA_XY_Create(GUI_GREEN, GUI_COUNTOF(Points), Points,GUI_COUNTOF(Points));
	//GRAPH_DATA_XY_SetPenSize(SineData, 2);
	//GRAPH_DATA_XY_SetLineVis(SineData, 0);
	//GRAPH_DATA_XY_SetPointVis(SineData, 1);
	GRAPH_AttachData(hItem, SineData);
}
