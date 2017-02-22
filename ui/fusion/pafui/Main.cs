using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Bespoke.Common.Osc;
using System.Collections;
using System.Net.Sockets;

namespace FusionUI
{
    public delegate void FJOscElementHandler(OscPacket element);

    public enum GUIDrawMode
    {
        NoGUIDraw = 0,
        WeightedVector2DGUIDraw = 1
    }

    [Flags]
    public enum WeightedVector2DGUIDrawModeAdditionalVectors
    {
        MassCenterVecDot = 2,
        MassCenterVecLine = 4,
        CombinedVecDot = 8,
        CombinedVecLine = 16,
        EntityVecDot = 32,
        EntityVecLine = 64
    }


    public partial class Main : Form
    {
        public Main()
        {
            System.Globalization.CultureInfo tmpCult = new System.Globalization.CultureInfo("en-US");
            //tmpCult.DateTimeFormat = new System.Globalization.DateTimeFormatInfo();
            tmpCult.DateTimeFormat = (System.Globalization.DateTimeFormatInfo)System.Threading.Thread.CurrentThread.CurrentCulture.DateTimeFormat.Clone();
            //System.Threading.Thread.CurrentThread.CurrentCulture.NumberFormat.CurrencyDecimalSeparator = "."; 
            System.Threading.Thread.CurrentThread.CurrentCulture = tmpCult;
            InitializeComponent();
        }

        private UDPReceiver UDPReceive;
        private FJOscElementHandler delegateToCallElementEvent;
        private object[] tempElementObject;
        //private Dictionary<string,  Entity> messagesSafe;
        private EntityContainer entityContainer;
        private float referenceFrameworkTime;
        private DateTime referenceGUITime;
        private DateTime curGUITime;
        //private TimeSpan tmpTimeSpan;
        private TimeSpan startTimeOffSet;
        private bool isTimeValid;
        private bool drawGridCartesianAutoSize;
        private AutoSizeMode drawGridCartesianAutoSizeMode;
        private Pen gridGrayPen = new Pen(Color.LightGray);
        private Pen gridBlackPen = new Pen(Color.Black);
        private Pen gridBlackDashPen = new Pen(Color.Black);
        private Pen greenLinePen = new Pen(Color.Green, 3.0f);
        private Pen blueLinePen = new Pen(Color.Blue, 3.0f);
        private Pen redLinePen = new Pen(Color.Red, 3.0f);
        private Font gridLegendFont = new Font("Arial", 10);
        private Brush fontBrush = new SolidBrush(Color.Black);
        private Brush bluePointBrush = new SolidBrush(Color.Blue);
        private Brush redPointBrush = new SolidBrush(Color.Red);
        private Brush greenPointBrush = new SolidBrush(Color.Green);
        private Size drawingDblBufferPanelClientSize;
        private Point drawingDblBufferPanelOrigin;
        //private Point drawingDblBufferPanelOriginScalable;
        private GUIDrawMode drawingMode = GUIDrawMode.NoGUIDraw;
        //private static int axisDrawingOffset = 30;
        private Point axisDrawingOffset;
        private RectangleF minMaxDrawingRegionScale;
        private WeightedVector2DGUIDrawModeAdditionalVectors weightedVector2DGUIDrawModeAdditionalVectors;
        private bool isInSpanningMode = false;
        private bool alfredControlActive = false;
        private System.Net.IPEndPoint alfredControlRemoteSocket;
        private PureDraw pureDrawWindow = null;
        private PointF upperLeftCornerOfSpanningSelectionInCartesian;
        private PointF upperLeftCornerOfSpanningSelectionInPixels;
        private PointF currentMousePositionInSpanningModeInCartesian;
        private PointF[] minAndMaxBoundariesOfCurrentCalculation = new PointF[2];
        private UdpClient udpClient;

        public WeightedVector2DGUIDrawModeAdditionalVectors WeightedVector2DGUIDrawModeAdditionalVectorsProp
        {
            get { return weightedVector2DGUIDrawModeAdditionalVectors; }
            set { weightedVector2DGUIDrawModeAdditionalVectors = value; }
        }

        public AutoSizeMode DrawGridartesianAutoSizeMode
        {
            get { return drawGridCartesianAutoSizeMode; }
            set { drawGridCartesianAutoSizeMode = value; }
        }

        public bool DrawGridCartesianAutoSize
        {
            get { return drawGridCartesianAutoSize; }
            set { drawGridCartesianAutoSize = value; }
        }

        public bool IsInSpanningMode
        {
            get { return isInSpanningMode; }
            set { isInSpanningMode = value; }
        }

        public PointF UpperLeftCornerOfSpanningSelection
        {
            get { return upperLeftCornerOfSpanningSelectionInCartesian; }
            set { upperLeftCornerOfSpanningSelectionInCartesian = value; }
        }

        public PointF CurrentMousePositionInSpanningMode
        {
            get { return currentMousePositionInSpanningModeInCartesian; }
            set { currentMousePositionInSpanningModeInCartesian = value; }
        }

        public void FJOscElementEventOnWorkerThread(OscPacket element)
        {
            tempElementObject[0] = element;
            Invoke(delegateToCallElementEvent, tempElementObject);
        }


        public void FJOscElementEventOnMainThread(OscPacket element)
        {            

            if (isTimeValid == false)
            {
                switch (element.Address)
                {
                    case "/strm":
                        //SyncTimes((float)element.Args[1]);
                        break;
                    case "/evnt":
                        //SyncTimes((float)element.Args[1]);
                        break;
                    case "/text":
                        //SyncTimes((float)element.Args[1]);
                        break;
                    default:
                        break;
                }
            }
            switch (element.Address)
            {
                case "/text":
                    if (element.Data[0].ToString() == "time_sync")
                    {
                        //SyncTimes((float)element.Args[1]);
                    }
                    break;
                case "/evnt":
                    CheckEnterId(element);
                    break;
                case "/strm":
                    break;
                default:
                    break;
            }
            
        }

        private void CheckEnterId(OscPacket element)
        {
            Type elementIdType = element.Data[0].GetType();
            Type stringType = Type.GetType("System.String");
           if (elementIdType == stringType)
            {
                curGUITime = DateTime.Now;
                string tmpKeyString = element.Data[0].ToString();
                Entity tmpNewEntity = new Entity(entityContainer);
                entityContainer.entitySafe.Add(tmpNewEntity);
                for (int i = 4; i < 4 + (int)element.Data[3] * 2; i = i + 2)
                {
                    tmpNewEntity[element.Data[i].ToString(), curGUITime] = (float)element.Data[i + 1];
                }               

                log_box.AppendText(element.Address);
                for (int i = 0; i < element.Data.Length; i++)
                {
                    log_box.AppendText(" " + element.Data[i]);
                }
                log_box.AppendText(Environment.NewLine);
            }
        }

        private void SyncTimes(float frameworkTime)
        {
            referenceGUITime = DateTime.Now - TimeSpan.FromSeconds(frameworkTime);
            referenceFrameworkTime = frameworkTime;
            isTimeValid = true;
            timer1.Enabled = true;
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            UDPReceive = new UDPReceiver();
            UDPReceive.FJUDPReceived += new FJOscElementHandler(FJOscElementEventOnWorkerThread);
            drawingMode = GUIDrawMode.WeightedVector2DGUIDraw;
            dblBufferedDrawControl1.OnPaintReceived += new DblBufferOnPaintEventHandler(drawWorks);
            delegateToCallElementEvent += FJOscElementEventOnMainThread;
            tempElementObject = new object[1];
            entityContainer = new EntityContainer();
            startTimeOffSet = TimeSpan.Zero;
            isTimeValid = false;
            drawGridCartesianAutoSizeMode = AutoSizeMode.GrowOnly;
            drawGridCartesianAutoSize = false;
            referenceFrameworkTime = 0.0f;
            drawingDblBufferPanelClientSize = dblBufferedDrawControl1.ClientSize;
            axisDrawingOffset = new Point(50, 50);
            minMaxDrawingRegionScale = new RectangleF(-1.0f, -1.0f, 2.0f, 2.0f);
            drawingDblBufferPanelClientSize.Width -= axisDrawingOffset.X;
            drawingDblBufferPanelClientSize.Height -= axisDrawingOffset.Y;
            drawingDblBufferPanelOrigin = new Point(drawingDblBufferPanelClientSize.Width / 2 + axisDrawingOffset.X, drawingDblBufferPanelClientSize.Height / 2);
            gridGrayPen.DashStyle = System.Drawing.Drawing2D.DashStyle.Dot;
            gridBlackDashPen.DashStyle = System.Drawing.Drawing2D.DashStyle.Dot;

            numericUpDown1.Value = (decimal)entityContainer.ContributionThreshold;
            numericUpDown2.Value = (decimal)entityContainer.Speed;
            numericUpDown3.Value = (decimal)timer1.Interval;

            WeightedVector2DGUIDrawModeAdditionalVectorsProp = WeightedVector2DGUIDrawModeAdditionalVectors.CombinedVecLine | WeightedVector2DGUIDrawModeAdditionalVectors.MassCenterVecDot | WeightedVector2DGUIDrawModeAdditionalVectors.EntityVecDot;
            checkBox1.CheckState = CheckState.Checked;
            checkBox2.CheckState = CheckState.Checked;
            checkBox3.CheckState = CheckState.Checked;
            checkBox4.CheckState = CheckState.Checked;
            checkBox5.CheckState = CheckState.Checked;
            checkBox6.CheckState = CheckState.Unchecked;
            SyncTimes(0.0f);

            alfredControlRemoteSocket = new System.Net.IPEndPoint(System.Net.IPAddress.Loopback, 5554);
            udpClient = new UdpClient();
            udpClient.Connect(alfredControlRemoteSocket);
            
        }

        

        public void drawWorks(DblBufferedDrawControl ctrl, Graphics graphics)
        {
            switch (drawingMode)
            {
                case GUIDrawMode.NoGUIDraw:
                    break;
                case GUIDrawMode.WeightedVector2DGUIDraw:
                    drawWorks2DWeightedArrowGUI(ctrl, graphics);
                    break;
                default:
                    break;
            }
        }

        private void calculationWorks2DWeightedArrowGUI()
        {
            entityContainer.overAllWeight = 0.0f;
            
            entityContainer.numberOfContributingVectors = 0;

            minAndMaxBoundariesOfCurrentCalculation[0].X = minAndMaxBoundariesOfCurrentCalculation[0].Y = float.MaxValue;
            minAndMaxBoundariesOfCurrentCalculation[1].X = minAndMaxBoundariesOfCurrentCalculation[1].Y = float.MinValue;

            for (int i = 0; i < 3; ++i)
            {
                entityContainer.modVec[i] = 0.0f;
                entityContainer.massCenter[i] = 0.0f;
            }

            List<Entity> entitiesToRemove = new List<Entity> ();
            foreach (Entity sePair in entityContainer.entitySafe)
            {
                if (sePair.IsEntityActive == true)
                {
                    entityContainer.decayFactor = sePair.CurrentDecayFactor;
                    entityContainer.tmpNorm = sePair.GetVectorNorm();
                    entityContainer.tmpDecayedVector[0] = sePair[0] * entityContainer.decayFactor / entityContainer.tmpNorm;
                    entityContainer.tmpDecayedVector[1] = sePair[1] * entityContainer.decayFactor / entityContainer.tmpNorm;
                    entityContainer.tmpDecayedVector[2] = sePair[2] * entityContainer.decayFactor / entityContainer.tmpNorm;
                    sePair.TmpXCoordinate = entityContainer.tmpDecayedVector[0];
                    sePair.TmpYCoordinate = entityContainer.tmpDecayedVector[1];
                    entityContainer.decayedNorm = Entity.GetVectorNorm(entityContainer.tmpDecayedVector[0], entityContainer.tmpDecayedVector[1]);
                    if (entityContainer.decayedNorm > entityContainer.ContributionThreshold)
                    {
                        entityContainer.overAllWeight += sePair.WeightFactor;
                        ++entityContainer.numberOfContributingVectors;
                        entityContainer.massCenter[0] += entityContainer.tmpDecayedVector[0] * sePair.WeightFactor;
                        entityContainer.massCenter[1] += entityContainer.tmpDecayedVector[1] * sePair.WeightFactor;

                        minAndMaxBoundariesOfCurrentCalculation[0].X = Math.Min(minAndMaxBoundariesOfCurrentCalculation[0].X, sePair.TmpXCoordinate);
                        minAndMaxBoundariesOfCurrentCalculation[0].Y = Math.Min(minAndMaxBoundariesOfCurrentCalculation[0].Y, sePair.TmpYCoordinate);
                        minAndMaxBoundariesOfCurrentCalculation[1].X = Math.Max(minAndMaxBoundariesOfCurrentCalculation[1].X, sePair.TmpXCoordinate);
                        minAndMaxBoundariesOfCurrentCalculation[1].Y = Math.Max(minAndMaxBoundariesOfCurrentCalculation[1].Y, sePair.TmpYCoordinate);
                    }
                    else
                    {
                        entitiesToRemove.Add (sePair);                        
                    }                   
                }
            }

            foreach (Entity sePair in entitiesToRemove) {
                entityContainer.entitySafe.Remove(sePair);
            }
            entitiesToRemove.Clear ();

            if (entityContainer.numberOfContributingVectors == 0)
            {
                for (int i = 0; i < 3; ++i)
                {
                    entityContainer.modVec[i] = -entityContainer.combinedVec[i];// *entityContainer.ModVecWeight;
                }
            }
            else
            {
                for (int i = 0; i < 3; ++i)
                {
                    entityContainer.massCenter[i] /= entityContainer.overAllWeight;
                    entityContainer.modVec[i] = (entityContainer.massCenter[i] - entityContainer.combinedVec[i]);// *entityContainer.ModVecWeight;
                }
            }

            float modVecNorm = Entity.GetVectorNorm(entityContainer.modVec[0], entityContainer.modVec[1], entityContainer.modVec[2]);

            float delta = (float)timer1.Interval / 1000.0f * entityContainer.Speed;

            if (modVecNorm > 0.0f)
            {
                delta = Math.Min(delta, modVecNorm);
                for (int i = 0; i < 3; ++i)
                {
                    entityContainer.modVec[i] = entityContainer.modVec[i] * (delta / modVecNorm);
                    entityContainer.combinedVec[i] += entityContainer.modVec[i];
                }
            }
            else
            {
                for (int i = 0; i < 3; ++i)
                {
                    entityContainer.modVec[i] = 0.0f;
                }
            }

            if (alfredControlActive == true)
            {
                controlAlfred();
            }

            minAndMaxBoundariesOfCurrentCalculation[0].X = Math.Min(minAndMaxBoundariesOfCurrentCalculation[0].X, entityContainer.combinedVec[0]);
            minAndMaxBoundariesOfCurrentCalculation[0].Y = Math.Min(minAndMaxBoundariesOfCurrentCalculation[0].Y, entityContainer.combinedVec[1]);
            minAndMaxBoundariesOfCurrentCalculation[1].X = Math.Max(minAndMaxBoundariesOfCurrentCalculation[1].X, entityContainer.combinedVec[0]);
            minAndMaxBoundariesOfCurrentCalculation[1].Y = Math.Max(minAndMaxBoundariesOfCurrentCalculation[1].Y, entityContainer.combinedVec[1]);

            minAndMaxBoundariesOfCurrentCalculation[0].X = Math.Min(minAndMaxBoundariesOfCurrentCalculation[0].X, entityContainer.massCenter[0]);
            minAndMaxBoundariesOfCurrentCalculation[0].Y = Math.Min(minAndMaxBoundariesOfCurrentCalculation[0].Y, entityContainer.massCenter[1]);
            minAndMaxBoundariesOfCurrentCalculation[1].X = Math.Max(minAndMaxBoundariesOfCurrentCalculation[1].X, entityContainer.massCenter[0]);
            minAndMaxBoundariesOfCurrentCalculation[1].Y = Math.Max(minAndMaxBoundariesOfCurrentCalculation[1].Y, entityContainer.massCenter[1]);

            if (drawGridCartesianAutoSize == true)
            {
                if (drawGridCartesianAutoSizeMode == AutoSizeMode.GrowOnly)
                {
                    RectangleF tmpRect = dblBufferedDrawControl1._paintHelper.SizeOfDrawingSpaceCartesian;
                    if (tmpRect.X > minAndMaxBoundariesOfCurrentCalculation[0].X ||
                        tmpRect.Y > minAndMaxBoundariesOfCurrentCalculation[0].Y ||
                        tmpRect.Right < minAndMaxBoundariesOfCurrentCalculation[1].X ||
                        tmpRect.Bottom < minAndMaxBoundariesOfCurrentCalculation[1].Y)
                    {
                        minAndMaxBoundariesOfCurrentCalculation[0].X = Math.Min(minAndMaxBoundariesOfCurrentCalculation[0].X, tmpRect.X);
                        minAndMaxBoundariesOfCurrentCalculation[0].Y = Math.Min(minAndMaxBoundariesOfCurrentCalculation[0].Y, tmpRect.Y);
                        minAndMaxBoundariesOfCurrentCalculation[1].X = Math.Max(minAndMaxBoundariesOfCurrentCalculation[1].X, tmpRect.Right);
                        minAndMaxBoundariesOfCurrentCalculation[1].Y = Math.Max(minAndMaxBoundariesOfCurrentCalculation[1].Y, tmpRect.Bottom);
                        dblBufferedDrawControl1.SetDrawingSurfaceRangeInCartesianCoordinates(new PointF(minAndMaxBoundariesOfCurrentCalculation[0].X, minAndMaxBoundariesOfCurrentCalculation[1].Y), new PointF(minAndMaxBoundariesOfCurrentCalculation[1].X, minAndMaxBoundariesOfCurrentCalculation[0].Y));

                        if (pureDrawWindow != null)
                        {
                            pureDrawWindow.ChangeCartesianGridBoundaries(new PointF(minAndMaxBoundariesOfCurrentCalculation[0].X, minAndMaxBoundariesOfCurrentCalculation[1].Y), new PointF(minAndMaxBoundariesOfCurrentCalculation[1].X, minAndMaxBoundariesOfCurrentCalculation[0].Y));
                        }
                    }
                }
            }

        }

        private void controlAlfred()
        {

            float p = entityContainer.combinedVec[0];
            float a = entityContainer.combinedVec[1];
            
            string emo_string = "PAD " + p + " " + a;
            byte[] data = Encoding.ASCII.GetBytes(emo_string);
            if(alfredControlActive == true)
                udpClient.Send(data, data.Length);
        }

        private void drawWorks2DWeightedArrowGUI(DblBufferedDrawControl ctrl, Graphics graphics)
        {
            graphics.Clear(Color.White);
                        
            ctrl._paintHelper.DrawGrid();
            
            
            ctrl._paintHelper.DrawCircle(new PointF(0.0f, 0.0f), entityContainer.ContributionThreshold, gridBlackPen);

            
            if ((WeightedVector2DGUIDrawModeAdditionalVectorsProp & WeightedVector2DGUIDrawModeAdditionalVectors.EntityVecLine) == WeightedVector2DGUIDrawModeAdditionalVectors.EntityVecLine)
            {
                foreach (Entity sePair in entityContainer.entitySafe)
                {
                    if (sePair.IsEntityActive == true)
                    {
                        ctrl._paintHelper.DrawVector(new PointF(sePair.TmpXCoordinate, sePair.TmpYCoordinate), blueLinePen);
                        
                    }
                }

            }

            if ((WeightedVector2DGUIDrawModeAdditionalVectorsProp & WeightedVector2DGUIDrawModeAdditionalVectors.EntityVecDot) == WeightedVector2DGUIDrawModeAdditionalVectors.EntityVecDot)
            {
                foreach (Entity sePair in entityContainer.entitySafe)
                {
                    if (sePair.IsEntityActive == true)
                    {
                        ctrl._paintHelper.DrawPointWithAnnotation(new PointF(sePair.TmpXCoordinate, sePair.TmpYCoordinate), bluePointBrush, sePair.IdName, bluePointBrush, gridLegendFont);
                    }
                }

            }

            if ((WeightedVector2DGUIDrawModeAdditionalVectorsProp & WeightedVector2DGUIDrawModeAdditionalVectors.CombinedVecLine) == WeightedVector2DGUIDrawModeAdditionalVectors.CombinedVecLine)
            {
                ctrl._paintHelper.DrawVector(new PointF(entityContainer.combinedVec[0], entityContainer.combinedVec[1]), redLinePen);
                
            }

            if ((WeightedVector2DGUIDrawModeAdditionalVectorsProp & WeightedVector2DGUIDrawModeAdditionalVectors.CombinedVecDot) == WeightedVector2DGUIDrawModeAdditionalVectors.CombinedVecDot)
            {
                ctrl._paintHelper.DrawPoint(new PointF(entityContainer.combinedVec[0], entityContainer.combinedVec[1]), redPointBrush);
                
            }

            if ((WeightedVector2DGUIDrawModeAdditionalVectorsProp & WeightedVector2DGUIDrawModeAdditionalVectors.MassCenterVecLine) == WeightedVector2DGUIDrawModeAdditionalVectors.MassCenterVecLine)
            {
                ctrl._paintHelper.DrawLine(new PointF(entityContainer.combinedVec[0], entityContainer.combinedVec[1]), new PointF(entityContainer.massCenter[0], entityContainer.massCenter[1]), greenLinePen);
                
            }

            if ((WeightedVector2DGUIDrawModeAdditionalVectorsProp & WeightedVector2DGUIDrawModeAdditionalVectors.MassCenterVecDot) == WeightedVector2DGUIDrawModeAdditionalVectors.MassCenterVecDot)
            {
                ctrl._paintHelper.DrawPoint(new PointF(entityContainer.massCenter[0], entityContainer.massCenter[1]), greenPointBrush);
                
            }

            if (isInSpanningMode == true)
            {
                DrawSpanningBox(ctrl);
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            switch (drawingMode)
            {
                case GUIDrawMode.NoGUIDraw:
                    break;
                case GUIDrawMode.WeightedVector2DGUIDraw:
                    calculationWorks2DWeightedArrowGUI();
                    break;
                default:
                    break;
            }
            if (undockChartToolStripMenuItem.CheckState == CheckState.Checked && pureDrawWindow != null)
            {
                pureDrawWindow.RefreshDrawingControl();
            }
            else
            {
                dblBufferedDrawControl1.Refresh();
            }
        }


        private void dblBufferedDrawControl1_ClientSizeChanged(object sender, EventArgs e)
        {
            drawingDblBufferPanelClientSize = dblBufferedDrawControl1.ClientSize;
            drawingDblBufferPanelClientSize.Width -= axisDrawingOffset.X;
            drawingDblBufferPanelClientSize.Height -= axisDrawingOffset.Y;
            drawingDblBufferPanelOrigin = new Point(drawingDblBufferPanelClientSize.Width / 2 + axisDrawingOffset.X, drawingDblBufferPanelClientSize.Height / 2);
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            gridGrayPen.Dispose();
            gridGrayPen = null;
            gridBlackPen.Dispose();
            gridBlackPen = null;
            gridBlackDashPen.Dispose();
            gridBlackDashPen = null;
            greenLinePen.Dispose();
            greenLinePen = null;
            blueLinePen.Dispose();
            blueLinePen = null;
            redLinePen.Dispose();
            redLinePen = null;
            fontBrush.Dispose();
            fontBrush = null;
            gridLegendFont.Dispose();
            gridLegendFont = null;
            bluePointBrush.Dispose();
            bluePointBrush = null;
            redPointBrush.Dispose();
            redPointBrush = null;
            greenPointBrush.Dispose();
            greenPointBrush = null;

            if (pureDrawWindow != null)
            {
                pureDrawWindow.Close();
            }

            udpClient.Close();
        }

        private void numericUpDownContributionThreshold_ValueChanged(object sender, EventArgs e)
        {
            entityContainer.ContributionThreshold = (float)numericUpDown1.Value;
        }

        private void listenportToolStripMenuItem_Click(object sender, EventArgs e)
        {
            PortSelectionForm tmpPortSelectionForm = new PortSelectionForm(UDPReceive.Port_number);
            tmpPortSelectionForm.ShowDialog();
            if (tmpPortSelectionForm.DialogResult == DialogResult.OK)
            {
                UDPReceive.Port_number = tmpPortSelectionForm.Port;
            }
            tmpPortSelectionForm.Dispose();
        }

        private void numericUpDown2_ValueChanged(object sender, EventArgs e)
        {
            entityContainer.Speed = (float)numericUpDown2.Value;
        }

        private void numericUpDown3_ValueChanged(object sender, EventArgs e)
        {
            timer1.Interval = (int)numericUpDown3.Value;
        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox1.CheckState == CheckState.Checked)
            {
                WeightedVector2DGUIDrawModeAdditionalVectorsProp = WeightedVector2DGUIDrawModeAdditionalVectorsProp | WeightedVector2DGUIDrawModeAdditionalVectors.MassCenterVecDot;
            }
            else
            {
                WeightedVector2DGUIDrawModeAdditionalVectorsProp = WeightedVector2DGUIDrawModeAdditionalVectorsProp ^ WeightedVector2DGUIDrawModeAdditionalVectors.MassCenterVecDot;
            }
        }

        private void checkBox2_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox2.CheckState == CheckState.Checked)
            {
                WeightedVector2DGUIDrawModeAdditionalVectorsProp = WeightedVector2DGUIDrawModeAdditionalVectorsProp | WeightedVector2DGUIDrawModeAdditionalVectors.MassCenterVecLine;
            }
            else
            {
                WeightedVector2DGUIDrawModeAdditionalVectorsProp = WeightedVector2DGUIDrawModeAdditionalVectorsProp ^ WeightedVector2DGUIDrawModeAdditionalVectors.MassCenterVecLine;
            }
        }

        private void checkBox3_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox3.CheckState == CheckState.Checked)
            {
                WeightedVector2DGUIDrawModeAdditionalVectorsProp = WeightedVector2DGUIDrawModeAdditionalVectorsProp | WeightedVector2DGUIDrawModeAdditionalVectors.CombinedVecDot;
            }
            else
            {
                WeightedVector2DGUIDrawModeAdditionalVectorsProp = WeightedVector2DGUIDrawModeAdditionalVectorsProp ^ WeightedVector2DGUIDrawModeAdditionalVectors.CombinedVecDot;
            }
        }

        private void checkBox4_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox4.CheckState == CheckState.Checked)
            {
                WeightedVector2DGUIDrawModeAdditionalVectorsProp = WeightedVector2DGUIDrawModeAdditionalVectorsProp | WeightedVector2DGUIDrawModeAdditionalVectors.CombinedVecLine;
            }
            else
            {
                WeightedVector2DGUIDrawModeAdditionalVectorsProp = WeightedVector2DGUIDrawModeAdditionalVectorsProp ^ WeightedVector2DGUIDrawModeAdditionalVectors.CombinedVecLine;
            }
        }

        private void checkBox5_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox5.CheckState == CheckState.Checked)
            {
                WeightedVector2DGUIDrawModeAdditionalVectorsProp = WeightedVector2DGUIDrawModeAdditionalVectorsProp | WeightedVector2DGUIDrawModeAdditionalVectors.EntityVecDot;
            }
            else
            {
                WeightedVector2DGUIDrawModeAdditionalVectorsProp = WeightedVector2DGUIDrawModeAdditionalVectorsProp ^ WeightedVector2DGUIDrawModeAdditionalVectors.EntityVecDot;
            }
        }

        private void checkBox6_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox6.CheckState == CheckState.Checked)
            {
                WeightedVector2DGUIDrawModeAdditionalVectorsProp = WeightedVector2DGUIDrawModeAdditionalVectorsProp | WeightedVector2DGUIDrawModeAdditionalVectors.EntityVecLine;
            }
            else
            {
                WeightedVector2DGUIDrawModeAdditionalVectorsProp = WeightedVector2DGUIDrawModeAdditionalVectorsProp ^ WeightedVector2DGUIDrawModeAdditionalVectors.EntityVecLine;
            }
        }

        private void createEntityToolStripMenuItem_Click(object sender, EventArgs e)
        {
            EntityCreatorForm tmpEntityCreatorForm = new EntityCreatorForm(entityContainer, "picked");
            tmpEntityCreatorForm.ShowDialog();

            if (tmpEntityCreatorForm.DialogResult == DialogResult.OK)
            {
                if (tmpEntityCreatorForm.Entity_id != String.Empty &&
                    tmpEntityCreatorForm.Entity_id != null &&
                    tmpEntityCreatorForm.EntityToCreate != null)
                {
                    entityContainer.entitySafe.Add(tmpEntityCreatorForm.EntityToCreate);            
                }
            }
            tmpEntityCreatorForm.Dispose();
        }

        private void dblBufferedDrawControl1_MouseClick(object sender, MouseEventArgs e)
        {

            if (e.Button == MouseButtons.Left)
            {
                float[] mouseSetCoords = {e.X, e.Y, 0.0f};
                if (sender.GetType() != typeof(DblBufferedDrawControl))
                {
                    return;
                }
                DblBufferedDrawControl ctrl = (DblBufferedDrawControl)sender;
                ctrl._paintHelper.TransformMouseCoordinatesIntoCartesianCoordinates(ref mouseSetCoords[0], ref mouseSetCoords[1]);

                curGUITime = DateTime.Now;
                string tmpKeyString = "picked";
                Entity tmpNewEntity = new Entity(entityContainer);
                entityContainer.entitySafe.Add(tmpNewEntity);
                for (int i = 0; i < 3 ; i++)
                {
                    tmpNewEntity[entityContainer.AxisMapName[i], curGUITime] = (float)mouseSetCoords[i];
                }

                log_box.AppendText(tmpKeyString);
                for (int i = 0; i < 3; i++)
                {
                    log_box.AppendText("\t" + (float)mouseSetCoords[i]);
                }
                log_box.AppendText(Environment.NewLine);
            }
        }

        private void dblBufferedDrawControl1_MouseLeave(object sender, EventArgs e)
        {
            this.Cursor = System.Windows.Forms.Cursors.Default;
            if (isInSpanningMode == true)
            {
                isInSpanningMode = false;
            }
        }

        private void alfredControlToolStripMenuItem_Click(object sender, EventArgs e)
        {
            bool alfredWasActiveControlled = false;
            if (alfredControlActive == true)
            {
                alfredControlActive = false;
                alfredWasActiveControlled = true;
            }

            SocketControlForm tmpAlfredControlForm = new SocketControlForm(alfredControlRemoteSocket);
            tmpAlfredControlForm.ShowDialog();
            if (tmpAlfredControlForm.DialogResult == DialogResult.OK)
            {
                alfredControlRemoteSocket = tmpAlfredControlForm.AlfredRemoteSocket;
            }
            tmpAlfredControlForm.Dispose();

            udpClient.Connect(alfredControlRemoteSocket);

            if (alfredWasActiveControlled == true)
            {
                alfredControlActive = true;
            }
            
        }

        private void alfredControlActiveToolStripMenuItem_CheckedChanged(object sender, EventArgs e)
        {
            if (alfredControlActiveToolStripMenuItem.CheckState == CheckState.Checked)
            {
                alfredControlActive = true;
            }
            else
            {
                alfredControlActive = false;
            }
        }

        public void RedockChartToolStripMenuItem()
        {
            if (undockChartToolStripMenuItem.CheckState == CheckState.Checked)
            {
                if (pureDrawWindow != null)
                {
                    if (pureDrawWindow.Visible == true)
                    {
                        undockChartToolStripMenuItem.Checked = false;
                    }
                }
            }
        }

        public void ChartFormClosed()
        {
            pureDrawWindow = null;
        }

        private void undockChartToolStripMenuItem_CheckedChanged(object sender, EventArgs e)
        {
            if (undockChartToolStripMenuItem.CheckState == CheckState.Checked)
            {
                if (pureDrawWindow == null)
                {
                    pureDrawWindow = new PureDraw(this, dblBufferedDrawControl1._paintHelper.SizeOfDrawingSpaceCartesian);
                    pureDrawWindow.Show();
                }
                else
                {
                    pureDrawWindow.Show();
                }
            }
            else
            {
                 if (pureDrawWindow != null)
                {
                    if (pureDrawWindow.Visible == true)
                    {
                        pureDrawWindow.Hide();
                    }
                }
            }
        }

        public void dblBufferedDrawControl1_MouseDown(object sender, MouseEventArgs e)
        {
            isInSpanningMode = true;
            upperLeftCornerOfSpanningSelectionInCartesian = e.Location;
            upperLeftCornerOfSpanningSelectionInPixels = e.Location;
            DblBufferedDrawControl ctrl = (DblBufferedDrawControl)sender;
            ctrl._paintHelper.TransformMouseCoordinatesIntoCartesianCoordinates(ref upperLeftCornerOfSpanningSelectionInCartesian);
            currentMousePositionInSpanningModeInCartesian = upperLeftCornerOfSpanningSelectionInCartesian;
        }

        public void dblBufferedDrawControl1_MouseUp(object sender, MouseEventArgs e)
        {
            if (isInSpanningMode == true)
            {
                isInSpanningMode = false;
                
                PointF lowerRightCornerOfSpanningSelectionInCartesian = e.Location;
                PointF lowerRightCornerOfSpanningSelectionInPixels = e.Location;

                if (upperLeftCornerOfSpanningSelectionInPixels.X > lowerRightCornerOfSpanningSelectionInPixels.X)
                {
                    if (upperLeftCornerOfSpanningSelectionInPixels.X - lowerRightCornerOfSpanningSelectionInPixels.X < 10.0f)
                    {
                        return;
                    }
                }
                else
                {
                    if (lowerRightCornerOfSpanningSelectionInPixels.X - upperLeftCornerOfSpanningSelectionInPixels.X < 10.0f)
                    {
                        return;
                    }
                }

                if (upperLeftCornerOfSpanningSelectionInPixels.Y < lowerRightCornerOfSpanningSelectionInPixels.Y)
                {
                    if (lowerRightCornerOfSpanningSelectionInPixels.Y - upperLeftCornerOfSpanningSelectionInPixels.Y < 10.0f)
                    {
                        return;
                    }
                }
                else
                {
                    if (upperLeftCornerOfSpanningSelectionInPixels.Y - lowerRightCornerOfSpanningSelectionInPixels.Y < 10.0f)
                    {
                        return;
                    }
                }

                drawGridCartesianAutoSize = false;
                DblBufferedDrawControl ctrl = (DblBufferedDrawControl)sender;
                ctrl._paintHelper.TransformMouseCoordinatesIntoCartesianCoordinates(ref lowerRightCornerOfSpanningSelectionInCartesian);

                
                if (upperLeftCornerOfSpanningSelectionInCartesian.X > lowerRightCornerOfSpanningSelectionInCartesian.X)
                {
                    float tmpVal = upperLeftCornerOfSpanningSelectionInCartesian.X;
                    upperLeftCornerOfSpanningSelectionInCartesian.X = lowerRightCornerOfSpanningSelectionInCartesian.X;
                    lowerRightCornerOfSpanningSelectionInCartesian.X = tmpVal;
                }

                if (upperLeftCornerOfSpanningSelectionInCartesian.Y < lowerRightCornerOfSpanningSelectionInCartesian.Y)
                {
                    float tmpVal = upperLeftCornerOfSpanningSelectionInCartesian.Y;
                    upperLeftCornerOfSpanningSelectionInCartesian.Y = lowerRightCornerOfSpanningSelectionInCartesian.Y;
                    lowerRightCornerOfSpanningSelectionInCartesian.Y = tmpVal;
                }


                if (ctrl == this.dblBufferedDrawControl1)
                {
                    ctrl.SetDrawingSurfaceRangeInCartesianCoordinates(upperLeftCornerOfSpanningSelectionInCartesian, lowerRightCornerOfSpanningSelectionInCartesian);
                }
                else
                {
                    dblBufferedDrawControl1.SetDrawingSurfaceRangeInCartesianCoordinates(upperLeftCornerOfSpanningSelectionInCartesian, lowerRightCornerOfSpanningSelectionInCartesian);
                }
                if (pureDrawWindow != null)
                {
                    pureDrawWindow.ChangeCartesianGridBoundaries(upperLeftCornerOfSpanningSelectionInCartesian, lowerRightCornerOfSpanningSelectionInCartesian);
                }
            }
        }

        private void DrawSpanningBox(DblBufferedDrawControl ctrl)
        {
            ctrl._paintHelper.DrawBox(upperLeftCornerOfSpanningSelectionInCartesian, currentMousePositionInSpanningModeInCartesian, gridBlackPen);
        }

        public void dblBufferedDrawControl1_MouseMove(object sender, MouseEventArgs e)
        {
            if (isInSpanningMode == true)
            {
                currentMousePositionInSpanningModeInCartesian = e.Location;
                DblBufferedDrawControl ctrl = (DblBufferedDrawControl)sender;
                ctrl._paintHelper.TransformMouseCoordinatesIntoCartesianCoordinates(ref currentMousePositionInSpanningModeInCartesian);
            }
        }

        private void setGridRangesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            GridRangeSettingForm gridRangeSettingsForm = new GridRangeSettingForm(this.dblBufferedDrawControl1._paintHelper.SizeOfDrawingSpaceCartesian, this.drawGridCartesianAutoSize, drawGridCartesianAutoSizeMode);
            gridRangeSettingsForm.ShowDialog();
            if(gridRangeSettingsForm.DialogResult == DialogResult.OK)
            {
                drawGridCartesianAutoSize = gridRangeSettingsForm._autoSizeAvailable;
                drawGridCartesianAutoSizeMode = gridRangeSettingsForm._autoSizeMode;
                if (gridRangeSettingsForm.numericUpDown1.Value > gridRangeSettingsForm.numericUpDown3.Value ||
                    gridRangeSettingsForm.numericUpDown2.Value > gridRangeSettingsForm.numericUpDown4.Value)
                {
                    //don't change anything
                }
                else
                {
                    dblBufferedDrawControl1.SetDrawingSurfaceRangeInCartesianCoordinates(
                        new PointF((float)gridRangeSettingsForm.numericUpDown1.Value, (float)gridRangeSettingsForm.numericUpDown4.Value),
                        new PointF((float)gridRangeSettingsForm.numericUpDown3.Value, (float)gridRangeSettingsForm.numericUpDown2.Value));

                    if (pureDrawWindow != null)
                    {
                        pureDrawWindow.ChangeCartesianGridBoundaries(
                            new PointF((float)gridRangeSettingsForm.numericUpDown1.Value, (float)gridRangeSettingsForm.numericUpDown4.Value),
                        new PointF((float)gridRangeSettingsForm.numericUpDown3.Value, (float)gridRangeSettingsForm.numericUpDown2.Value));
                    }
                }
            }
            gridRangeSettingsForm.Dispose();
            gridRangeSettingsForm = null;
        }
    }
}
