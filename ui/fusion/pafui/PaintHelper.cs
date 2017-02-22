using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Drawing.Drawing2D;

namespace FusionUI
{
    public class PaintHelper : IDisposable
    {
        #region Member variables
        private Graphics _graphicsObject = null;
        private Size _sizeOfDrawingSurfaceInPixels;
        private RectangleF _sizeOfDrawingSpaceCartesian;
        private float _gridDrawSpacingStepsY;
        private float _gridDrawSpacingStepsX;
        private float _minDistanceBetweenGridLinesInPixels;
        private SizeF _currentDistanceBetweenGridLinesInPixels;
        private PointF[][] _coordinatesForYAxisGridLinesInCartesian;
        private PointF[][] _coordinatesForXAxisGridLinesInCartesian;
        private PointF[][] _coordinatesForYAxisGridLinesInPixels;
        private PointF[][] _coordinatesForXAxisGridLinesInPixels;
        private int _numberOfShiftsY;
        private int _numberOfShiftsX;
        private Font _gridLegendFont = null;
        private Size _offsetForGridAnnotation = new Size(0, 0);
        PointF[] _coordinatesForGridLegendY = null;
        PointF[] _coordinatesForGridLegendX = null;

        private Matrix _transformationMatrix = null;
        private Matrix _yAxisSwitchMatrix = null;
        private Matrix _inverseTransformationMatrix = null;
        private PointF _originOfGridInPixels;
        private PointF _originOfGridInCartesian;

        private Pen _gridGrayDashedPen = null;
        private Pen _gridBlackDashedPen = null;
        private Pen _blueLinePen = null;
        private Brush _gridLegendFontBrush = null;


        private bool isDisposed = false;

        #endregion


        #region Public Accessor Functions and Properties

        public Graphics GraphicsObjectContext
        {
            get { return _graphicsObject; }
            //set { _graphicsObject = value; }
        }

        public Size SizeOfDrawingSurfaceInPixels
        {
            get { return _sizeOfDrawingSurfaceInPixels; }
            //set { _sizeOfDrawingSurfaceInPixels = value; }
        }

        public RectangleF SizeOfDrawingSpaceCartesian
        {
            get { return _sizeOfDrawingSpaceCartesian; }
            //set { _sizeOfDrawingSpaceCartesian = value; }
        }

        #endregion

        ~PaintHelper()
        {
            Dispose(false);
        }

        public PaintHelper()
        {
        }

        public PaintHelper(Graphics graphicsContext, Size sizeOfDrawingSurfaceInPixels, RectangleF sizeOfDrawingSpaceCartesian) :
            this(graphicsContext, sizeOfDrawingSurfaceInPixels, sizeOfDrawingSpaceCartesian, false)
        {
        }

        public PaintHelper(Graphics graphicsContext, Size sizeOfDrawingSurfaceInPixels, RectangleF sizeOfDrawingSpaceCartesian, bool gridCompensation)
        {
            _gridGrayDashedPen = new Pen(Color.LightGray);
            _gridGrayDashedPen.DashStyle = DashStyle.Dot;
            _gridBlackDashedPen = new Pen(Color.Black);
            _gridBlackDashedPen.DashStyle = DashStyle.Dot;
            _blueLinePen = new Pen(Color.Blue, 3.0f);
            _gridLegendFontBrush = new SolidBrush(Color.Black);

            _graphicsObject = graphicsContext;
            _sizeOfDrawingSurfaceInPixels = sizeOfDrawingSurfaceInPixels;
            _sizeOfDrawingSpaceCartesian = sizeOfDrawingSpaceCartesian;
            _gridDrawSpacingStepsY = 1.0f;
            _gridDrawSpacingStepsX = 1.0f;
            _minDistanceBetweenGridLinesInPixels = 20;
            _yAxisSwitchMatrix = new Matrix(1.0f, 0.0f, 0.0f, -1.0f, 0.0f, _sizeOfDrawingSurfaceInPixels.Height);
            _originOfGridInCartesian = new PointF(0.0f, 0.0f);

            int numberOfShifts = 0;
            decimal remainder = 0.0m;
            float x = 0.0f;
            float width = 0.0f;
            float y = 0.0f;
            float height = 0.0f;
            int numberOfGridLinesInYDirection = 0;
            int numberOfGridLinesInXDirection = 0;
            float tempCalcFloat = 0.0f;

            //Width and X
            width = sizeOfDrawingSpaceCartesian.Width;

            if (Math.Abs(width) <= 1.0f)
            {
                numberOfShifts = 0;
                while (Math.Abs(width) <= 1.0f)
                {
                    width *= 10.0f;
                    ++numberOfShifts;
                    _gridDrawSpacingStepsY /= 10.0f;
                }
                

                if (gridCompensation == true)
                {
                    float tmpVal = width;
                    tmpVal -= (float)Math.Truncate(width);
                    tmpVal *= 10.0f;
                    tmpVal = (float)Math.Truncate(tmpVal);
                    if (tmpVal < 5.0f)
                    {
                        width += 0.1f;
                        width *= 10.0f;
                        width = (float)Math.Truncate(width);
                        width /= 10.0f;
                    }
                    else
                    {
                        width += 1.0f;
                        width = (float)Math.Truncate(width);
                    }
                }
                if (width <= 5.0f)
                {
                    _gridDrawSpacingStepsY /= 10.0f;
                }
                _numberOfShiftsY = numberOfShifts;
                while (numberOfShifts > 0)
                {
                    width /= 10.0f;
                    --numberOfShifts;
                }
            }
            else
            {
                numberOfShifts = 0;
                while (Math.Abs(width) > 10.0f)
                {
                    width /= 10.0f;
                    --numberOfShifts;
                    _gridDrawSpacingStepsY *= 10.0f;
                }
                if (gridCompensation == true)
                {
                    float tmpVal = width;
                    tmpVal -= (float)Math.Truncate(width);
                    tmpVal *= 10.0f;
                    tmpVal = (float)Math.Truncate(tmpVal);
                    if (tmpVal < 5.0f)
                    {
                        width += 0.1f;
                        width *= 10.0f;
                        width = (float)Math.Truncate(width);
                        width /= 10.0f;
                    }
                    else
                    {
                        width += 1.0f;
                        width = (float)Math.Truncate(width);
                    }
                }
                if (width <= 5.0f)
                {
                    _gridDrawSpacingStepsY /= 10.0f;
                }
                
                _numberOfShiftsY = numberOfShifts;
                while (numberOfShifts < 0)
                {
                    width *= 10.0f;
                    ++numberOfShifts;
                }
            }
            tempCalcFloat = width / _gridDrawSpacingStepsY;
            numberOfGridLinesInYDirection = (int)Math.Truncate(tempCalcFloat);
            if (sizeOfDrawingSurfaceInPixels.Width / numberOfGridLinesInYDirection < _minDistanceBetweenGridLinesInPixels)
            {
                tempCalcFloat = width / (_gridDrawSpacingStepsY / 2.0f);
                if (sizeOfDrawingSurfaceInPixels.Width / (int)Math.Truncate(tempCalcFloat) < _minDistanceBetweenGridLinesInPixels)
                {
                    //5-step
                    _gridDrawSpacingStepsY *= 5.0f;
                    tempCalcFloat = width / _gridDrawSpacingStepsY;
                    numberOfGridLinesInYDirection = (int)Math.Truncate(tempCalcFloat);
                }
                else
                {
                    //2-step
                    _gridDrawSpacingStepsY *= 2.0f;
                    tempCalcFloat = width / _gridDrawSpacingStepsY;
                    numberOfGridLinesInYDirection = (int)Math.Truncate(tempCalcFloat);
                }
            }

            x = sizeOfDrawingSpaceCartesian.X - (width - sizeOfDrawingSpaceCartesian.Width) / 2.0f;

            _coordinatesForYAxisGridLinesInCartesian = new PointF[numberOfGridLinesInYDirection][];
            _coordinatesForYAxisGridLinesInPixels = new PointF[numberOfGridLinesInYDirection][];

            for (int i = 0; i < _coordinatesForYAxisGridLinesInCartesian.GetLength(0); ++i)
            {
                _coordinatesForYAxisGridLinesInCartesian[i] = new PointF[2];
                _coordinatesForYAxisGridLinesInPixels[i] = new PointF[2];
            }

            remainder = (decimal)x % (decimal)_gridDrawSpacingStepsY;

            decimal startGridLineInYDirection = ((decimal)x + Math.Abs(remainder));


            //Height and Y
            height = sizeOfDrawingSpaceCartesian.Height;
            if (Math.Abs(height) <= 1.0f)
            {
                numberOfShifts = 0;
                while (Math.Abs(height) < 1.0f)
                {
                    height *= 10.0f;
                    ++numberOfShifts;
                    _gridDrawSpacingStepsX /= 10.0f;
                }
                
                if (gridCompensation == true)
                {
                    float tmpVal = height;
                    tmpVal -= (float)Math.Truncate(height);
                    tmpVal *= 10.0f;
                    tmpVal = (float)Math.Truncate(tmpVal);
                    if (tmpVal < 5.0f)
                    {
                        height += 0.1f;
                        height *= 10.0f;
                        height = (float)Math.Truncate(height);
                        height /= 10.0f;
                    }
                    else
                    {
                        height += 1.0f;
                        height = (float)Math.Truncate(height);
                    }
                    
                }
                if (height <= 5.0f)
                {
                    _gridDrawSpacingStepsX /= 10.0f;
                }
                _numberOfShiftsX = numberOfShifts;
                while (numberOfShifts > 0)
                {
                    height /= 10.0f;
                    --numberOfShifts;
                }
            }
            else
            {
                numberOfShifts = 0;
                while (Math.Abs(height) > 10.0f)
                {
                    height /= 10.0f;
                    --numberOfShifts;
                    _gridDrawSpacingStepsX *= 10.0f;
                }
                if (gridCompensation == true)
                {
                    float tmpVal = height;
                    tmpVal -= (float)Math.Truncate(height);
                    tmpVal *= 10.0f;
                    tmpVal = (float)Math.Truncate(tmpVal);
                    if (tmpVal < 5.0f)
                    {
                        height += 0.1f;
                        height *= 10.0f;
                        height = (float)Math.Truncate(height);
                        height /= 10.0f;
                    }
                    else
                    {
                        height += 1.0f;
                        height = (float)Math.Truncate(height);
                    }
                }
                if (height <= 5.0f)
                {
                    _gridDrawSpacingStepsX /= 10.0f;
                }
                _numberOfShiftsX = numberOfShifts;
                while (numberOfShifts < 0)
                {
                    height *= 10.0f;
                    ++numberOfShifts;
                }
            }
            tempCalcFloat = height / _gridDrawSpacingStepsX;
            numberOfGridLinesInXDirection = (int)Math.Truncate(tempCalcFloat);
            if (sizeOfDrawingSurfaceInPixels.Height / numberOfGridLinesInXDirection < _minDistanceBetweenGridLinesInPixels)
            {
                tempCalcFloat = height / (_gridDrawSpacingStepsX / 2.0f);
                if (sizeOfDrawingSurfaceInPixels.Height / (int)Math.Truncate(tempCalcFloat) < _minDistanceBetweenGridLinesInPixels)
                {
                    //5-step
                    _gridDrawSpacingStepsX *= 5.0f;
                    tempCalcFloat = height / _gridDrawSpacingStepsX;
                    numberOfGridLinesInXDirection = (int)Math.Truncate(tempCalcFloat);
                }
                else
                {
                    //2-step
                    _gridDrawSpacingStepsX *= 2.0f;
                    tempCalcFloat = height / _gridDrawSpacingStepsX;
                    numberOfGridLinesInXDirection = (int)Math.Truncate(tempCalcFloat);
                }
            }
            y = sizeOfDrawingSpaceCartesian.Y - (height - sizeOfDrawingSpaceCartesian.Height) / 2.0f;

            _coordinatesForXAxisGridLinesInCartesian = new PointF[numberOfGridLinesInXDirection][];
            _coordinatesForXAxisGridLinesInPixels = new PointF[numberOfGridLinesInXDirection][];

            for (int i = 0; i < _coordinatesForXAxisGridLinesInCartesian.GetLength(0); ++i)
            {
                _coordinatesForXAxisGridLinesInCartesian[i] = new PointF[2];
                _coordinatesForXAxisGridLinesInPixels[i] = new PointF[2];
            }

            remainder = (decimal)y % (decimal)_gridDrawSpacingStepsX;
            decimal startGridLineInXDirection = ((decimal)y + Math.Abs(remainder));

            for (int i = 0; i < numberOfGridLinesInYDirection; ++i)
            {
                _coordinatesForYAxisGridLinesInCartesian[i][0] = new PointF((float)(startGridLineInYDirection + (decimal)i * (decimal)_gridDrawSpacingStepsY), y);
                _coordinatesForYAxisGridLinesInCartesian[i][1] = new PointF((float)(startGridLineInYDirection + (decimal)i * (decimal)_gridDrawSpacingStepsY), y + height);
            }


            for (int i = 0; i < numberOfGridLinesInXDirection; ++i)
            {
                _coordinatesForXAxisGridLinesInCartesian[i][0] = new PointF(x, (float)(startGridLineInXDirection + (decimal)i * (decimal)_gridDrawSpacingStepsX));
                _coordinatesForXAxisGridLinesInCartesian[i][1] = new PointF(x + width, (float)(startGridLineInXDirection + (decimal)i * (decimal)_gridDrawSpacingStepsX));
            }

            _sizeOfDrawingSpaceCartesian = new RectangleF(x, y, width, height);

            Font tempFont = new Font("Arial", 10.0f);
            ChangeGridLegendFont(tempFont);

            TransformGridCoordinates();
            _currentDistanceBetweenGridLinesInPixels = new SizeF(Math.Abs(_coordinatesForYAxisGridLinesInPixels[1][0].X - _coordinatesForYAxisGridLinesInPixels[0][0].X),
                Math.Abs(_coordinatesForXAxisGridLinesInPixels[1][0].Y - _coordinatesForXAxisGridLinesInPixels[0][0].Y));
            TransformAnnotationCoordinates();

        }

        private void DetermineOriginInPixels()
        {
            Rectangle tempRect = new Rectangle(new Point(_offsetForGridAnnotation.Width, 0), new Size(_sizeOfDrawingSurfaceInPixels.Width - _offsetForGridAnnotation.Width, _sizeOfDrawingSurfaceInPixels.Height - _offsetForGridAnnotation.Height));
            decimal TX;
            decimal TY;
            if (Math.Abs(_sizeOfDrawingSpaceCartesian.Right) > 2.0f * float.Epsilon)
            {
                decimal teilVerhältnisX = (decimal)-_sizeOfDrawingSpaceCartesian.X / (decimal)_sizeOfDrawingSpaceCartesian.Right;
                TX = teilVerhältnisX * (decimal)tempRect.Width / (1.0m + teilVerhältnisX);
            }
            else
            {
                TX = _sizeOfDrawingSurfaceInPixels.Width - _offsetForGridAnnotation.Width;
            }

            if (Math.Abs(_sizeOfDrawingSpaceCartesian.Bottom) > 2.0f * float.Epsilon)
            {
                decimal teilVerhältnisY = (decimal)-_sizeOfDrawingSpaceCartesian.Y / (decimal)_sizeOfDrawingSpaceCartesian.Bottom;
                TY = teilVerhältnisY * (decimal)tempRect.Height / (1.0m + teilVerhältnisY);
            }
            else
            {
                TY = _sizeOfDrawingSurfaceInPixels.Height - _offsetForGridAnnotation.Height;
            }

            _originOfGridInPixels = new PointF((float)TX + _offsetForGridAnnotation.Width, (float)TY);
            decimal scaleFactorX = (decimal)tempRect.Width / (decimal)_sizeOfDrawingSpaceCartesian.Width;
            decimal scaleFactorY = (decimal)tempRect.Height / (decimal)_sizeOfDrawingSpaceCartesian.Height;

            _transformationMatrix = new Matrix((float)scaleFactorX, 0.0f, 0.0f, (float)scaleFactorY, _originOfGridInPixels.X, _originOfGridInPixels.Y);
            _transformationMatrix.Multiply(_yAxisSwitchMatrix, MatrixOrder.Append);
            _inverseTransformationMatrix = _transformationMatrix.Clone();
            _inverseTransformationMatrix.Invert();
        }

        /// <summary>
        /// Set a new Font for the Annotation.
        /// </summary>
        /// <param name="gridLegendFont">The Font will be disposed by this instance after usage. Do not pass a Font that will be used somewhere else or Dispose() it yourself</param>
        public void ChangeGridLegendFont(Font gridLegendFont)
        {
            if (_gridLegendFont != null)
            {
                _gridLegendFont.Dispose();
                _gridLegendFont = null;
            }
            _gridLegendFont = gridLegendFont;
            float tempWidth = 0.0f;
            float tempHeight = 0.0f;
            _coordinatesForGridLegendY = new PointF[_coordinatesForXAxisGridLinesInCartesian.GetLength(0)];
            _coordinatesForGridLegendX = new PointF[_coordinatesForYAxisGridLinesInCartesian.GetLength(0)];
            SizeF tempSize;
            for (int i = 0; i < _coordinatesForYAxisGridLinesInCartesian.GetLength(0); ++i)
            {
                _coordinatesForGridLegendX[i] = _coordinatesForYAxisGridLinesInCartesian[i][0];
                tempSize = _graphicsObject.MeasureString(_coordinatesForYAxisGridLinesInCartesian[i][0].X.ToString(), _gridLegendFont);
                if (tempWidth < tempSize.Width)
                {
                    tempWidth = tempSize.Width;
                }
            }
            
            for (int i = 0; i < _coordinatesForXAxisGridLinesInCartesian.GetLength(0); ++i)
            {
                _coordinatesForGridLegendY[i] = _coordinatesForXAxisGridLinesInCartesian[i][0];
                tempSize = _graphicsObject.MeasureString(_coordinatesForXAxisGridLinesInCartesian[i][0].Y.ToString(), _gridLegendFont);
                if (tempHeight < tempSize.Height)
                {
                    tempHeight = tempSize.Height;
                }
            }
            _offsetForGridAnnotation = new Size(2 + (int)tempWidth, 2 + (int)tempHeight);

            _yAxisSwitchMatrix.Translate(0.0f, (float)_offsetForGridAnnotation.Height);

            DetermineOriginInPixels();

        }

        private void TransformGridCoordinates()
        {
            for (int i = 0; i < _coordinatesForYAxisGridLinesInCartesian.GetLength(0); ++i)
            {
                _coordinatesForYAxisGridLinesInPixels[i][0] = new PointF(_coordinatesForYAxisGridLinesInCartesian[i][0].X, _coordinatesForYAxisGridLinesInCartesian[i][0].Y);
                _coordinatesForYAxisGridLinesInPixels[i][1] = new PointF(_coordinatesForYAxisGridLinesInCartesian[i][1].X, _coordinatesForYAxisGridLinesInCartesian[i][1].Y);
            }
            for (int i = 0; i < _coordinatesForXAxisGridLinesInCartesian.GetLength(0); ++i)
            {
                _coordinatesForXAxisGridLinesInPixels[i][0] = new PointF(_coordinatesForXAxisGridLinesInCartesian[i][0].X, _coordinatesForXAxisGridLinesInCartesian[i][0].Y);
                _coordinatesForXAxisGridLinesInPixels[i][1] = new PointF(_coordinatesForXAxisGridLinesInCartesian[i][1].X, _coordinatesForXAxisGridLinesInCartesian[i][1].Y);
            }
            
            int xLines = _coordinatesForYAxisGridLinesInCartesian.GetLength(0);
            for (int i = 0; i < xLines; ++i)
            {
                _transformationMatrix.TransformPoints(_coordinatesForYAxisGridLinesInPixels[i]);
            }

            int yLines = _coordinatesForXAxisGridLinesInCartesian.GetLength(0);
            for (int i = 0; i < yLines; ++i)
            {
                _transformationMatrix.TransformPoints(_coordinatesForXAxisGridLinesInPixels[i]);
            }
        }

        private void TransformAnnotationCoordinates()
        {
            _transformationMatrix.TransformPoints(_coordinatesForGridLegendY);
            _transformationMatrix.TransformPoints(_coordinatesForGridLegendX);
            int xLines = _coordinatesForGridLegendY.GetLength(0);
            for (int i = 0; i < xLines; ++i)
            {
                _coordinatesForGridLegendY[i] = new PointF(0, _coordinatesForGridLegendY[i].Y -
                    _graphicsObject.MeasureString(_coordinatesForXAxisGridLinesInCartesian[i][0].Y.ToString(), _gridLegendFont).Height / 2.0f);
            }

            int yLines = _coordinatesForGridLegendX.GetLength(0);
            for (int i = 0; i < yLines; ++i)
            {
                _coordinatesForGridLegendX[i] = new PointF(_coordinatesForGridLegendX[i].X -
                    _graphicsObject.MeasureString(_coordinatesForYAxisGridLinesInCartesian[i][1].X.ToString(), _gridLegendFont).Width / 2.0f , _sizeOfDrawingSurfaceInPixels.Height - _graphicsObject.MeasureString(_coordinatesForYAxisGridLinesInCartesian[i][1].X.ToString(), _gridLegendFont).Height);
            }
        }

        public void DrawGrid()
        {
            _graphicsObject.Clear(Color.White);
            int xLines = _coordinatesForYAxisGridLinesInCartesian.GetLength(0);
            
            int numberToSkip = 0;
            float temp;
            Pen colorToUse = null;
            for (int i = 0; i < xLines; ++i)
            {
                if (/*i == 0 ||*/ i == xLines || (Math.Abs(_coordinatesForYAxisGridLinesInCartesian[i][0].X) <= 2.0f * float.Epsilon))
                {
                    colorToUse = Pens.Black;
                }
                else
                {
                    colorToUse = _gridGrayDashedPen;
                }
                DrawLineWithoutTransformation(_coordinatesForYAxisGridLinesInPixels[i][0], _coordinatesForYAxisGridLinesInPixels[i][1], colorToUse);
                if (numberToSkip == 0)
                {
                    _graphicsObject.DrawString(_coordinatesForYAxisGridLinesInCartesian[i][0].X.ToString(), _gridLegendFont, _gridLegendFontBrush, _coordinatesForGridLegendX[i]);
                    temp = _graphicsObject.MeasureString(_coordinatesForYAxisGridLinesInCartesian[i][0].X.ToString(), _gridLegendFont).Width + 0.0f;
                    numberToSkip = (int)(temp / _currentDistanceBetweenGridLinesInPixels.Width);
                }
                else
                {
                    --numberToSkip;
                }
                
            }

            numberToSkip = 0;
            int yLines = _coordinatesForXAxisGridLinesInCartesian.GetLength(0);
            for (int i = 0; i < yLines; ++i)
            {
                if (/*i == 0 ||*/ i == yLines || (Math.Abs(_coordinatesForXAxisGridLinesInCartesian[i][0].Y) <= 2.0f * float.Epsilon))
                {
                    colorToUse = Pens.Black;
                }
                else
                {
                    colorToUse = _gridGrayDashedPen;
                }
                DrawLineWithoutTransformation(_coordinatesForXAxisGridLinesInPixels[i][0], _coordinatesForXAxisGridLinesInPixels[i][1], colorToUse);
                if (numberToSkip == 0)
                {
                    _graphicsObject.DrawString(_coordinatesForXAxisGridLinesInCartesian[i][0].Y.ToString(), _gridLegendFont, _gridLegendFontBrush, _coordinatesForGridLegendY[i]);
                    temp = _graphicsObject.MeasureString(_coordinatesForXAxisGridLinesInCartesian[i][0].Y.ToString(), _gridLegendFont).Height;
                    numberToSkip = (int)(temp / _currentDistanceBetweenGridLinesInPixels.Height);
                }
                else
                {
                    --numberToSkip;
                }
            }

            //DrawPoint(new PointF(0.0f, 0.0f), Brushes.Blue);
            //DrawPoint(new PointF(-1.0f, -1.0f), Brushes.Blue);
            //DrawCircle(new PointF(0.0f, 0.0f), 0.1f, Pens.Black);
        }

        public void DrawLineWithoutTransformation(PointF startPoint, PointF endPoint, Pen linePen)
        {
            DrawLine(startPoint, endPoint, linePen, false);
        }

        private void DrawLine(PointF startPoint, PointF endPoint, Pen linePen, bool doTransformation)
        {
            if (doTransformation == true)
            {
                PointF[] tmpPoint = new PointF[2];
                tmpPoint[0] = new PointF(startPoint.X, startPoint.Y);
                tmpPoint[1] = new PointF(endPoint.X, endPoint.Y);
                _transformationMatrix.TransformPoints(tmpPoint);
                _graphicsObject.DrawLine(linePen, tmpPoint[0], tmpPoint[1]);
            }
            else
            {
                _graphicsObject.DrawLine(linePen, startPoint, endPoint);
            }
            
            
        }

        public void DrawLine(PointF startPoint, PointF endPoint, Pen linePen)
        {
            DrawLine(startPoint, endPoint, linePen, true);
        }

        public void DrawVector(PointF vector, Pen vectorPen)
        {
            DrawLine(_originOfGridInCartesian, vector, vectorPen, true);
        }


        public void DrawPointWithoutTransformation(PointF point, Brush pointBrush)
        {
            DrawPoint(point, pointBrush, false);
        }

        private PointF DrawPoint(PointF point, Brush pointBrush, bool doTransformation)
        {
            if (doTransformation == true)
            {
                PointF[] tmpPoint = new PointF[1];
                tmpPoint[0] = new PointF(point.X, point.Y);
                _transformationMatrix.TransformPoints(tmpPoint);
                _graphicsObject.FillEllipse(pointBrush, tmpPoint[0].X-3.0f, tmpPoint[0].Y-3.0f, 6.0f, 6.0f);
                return tmpPoint[0];
            }

            else
            {
                _graphicsObject.FillEllipse(pointBrush, point.X-3.0f, point.Y-3.0f, 6.0f, 6.0f);
                return point;
            }
        }

        public void DrawPoint(PointF point, Brush pointBrush)
        {
            DrawPoint(point, pointBrush, true);
        }

        public void DrawPointWithAnnotation(PointF point, Brush pointBrush, string pointId, Brush annotationBrush, Font annotationFont)
        {
            PointF tmpPoint = DrawPoint(point, pointBrush, true);
            tmpPoint.X  = tmpPoint.X + 3.0f;
            tmpPoint.Y = tmpPoint.Y - 3.0f;
            _graphicsObject.DrawString(pointId, annotationFont, annotationBrush, tmpPoint);
        }

        public void DrawCircle(PointF center, float radius, Pen ellipsePen)
        {
            PointF[] ellipse = new PointF[2];
            ellipse[0].X = center.X - radius;
            ellipse[0].Y = center.Y - radius;
            ellipse[1].X = center.X + radius;
            ellipse[1].Y = center.Y + radius;
            _transformationMatrix.TransformPoints(ellipse);
            SizeF size = new SizeF(ellipse[1].X - ellipse[0].X, ellipse[1].Y - ellipse[0].Y);
            RectangleF rect = new RectangleF(ellipse[0], size);
            _graphicsObject.DrawEllipse(ellipsePen, rect);

        }

        public void DrawBox(PointF upperLeftCorner, PointF lowerRightCorner, Pen boxPen)
        {
            PointF[] points = new PointF[4];
            points[0] = upperLeftCorner;
            points[1] = new PointF(lowerRightCorner.X, upperLeftCorner.Y);
            points[2] = lowerRightCorner;
            points[3] = new PointF(upperLeftCorner.X, lowerRightCorner.Y);
            _transformationMatrix.TransformPoints(points);
            DrawLine(points[0], points[1], boxPen, false);
            DrawLine(points[1], points[2], boxPen, false);
            DrawLine(points[2], points[3], boxPen, false);
            DrawLine(points[3], points[0], boxPen, false);
        }

        public void TransformMouseCoordinatesIntoCartesianCoordinates(ref float mouseX, ref float mouseY)
        {
            PointF[] mouse = new PointF[1];
            mouse[0] = new PointF(mouseX, mouseY);
            _inverseTransformationMatrix.TransformPoints(mouse);
            mouseX = mouse[0].X;
            mouseY = mouse[0].Y;
        }

        public void TransformMouseCoordinatesIntoCartesianCoordinates(ref PointF mouseCoords)
        {
            PointF[] mouse = new PointF[1];
            mouse[0] = mouseCoords;
            _inverseTransformationMatrix.TransformPoints(mouse);
            mouseCoords = mouse[0];
        }

        #region IDisposable Members

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!isDisposed)
            {
                if (disposing)
                {
                    //Dispose managed ressources
                    if (_gridGrayDashedPen != null)
                    {
                        _gridGrayDashedPen.Dispose();
                        _gridGrayDashedPen = null;
                    }
                    if (_gridBlackDashedPen != null)
                    {
                        _gridBlackDashedPen.Dispose();
                        _gridBlackDashedPen = null;
                    }
                    if (_gridLegendFont != null)
                    {
                        _gridLegendFont.Dispose();
                        _gridLegendFont = null;
                    }
                    if (_blueLinePen != null)
                    {
                        _blueLinePen.Dispose();
                        _blueLinePen = null;
                    }
                    if (_gridLegendFontBrush != null)
                    {
                        _gridLegendFontBrush.Dispose();
                        _gridLegendFontBrush = null;
                    }
                }

                isDisposed = true;
                //base.Dispose(disposing);
            }
            
        }
        #endregion

    }
}
