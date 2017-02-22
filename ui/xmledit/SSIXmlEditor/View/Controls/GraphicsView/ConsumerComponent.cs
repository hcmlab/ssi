using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Drawing.Drawing2D;

namespace SSIXmlEditor.Controls.GraphicsView
{
    class ConsumerComponent : GraphicsComponent
    {
        private List<InputComponent> m_Inputs;

		public IEnumerable<InputComponent> Inputs { get { return m_Inputs; } }

        public ConsumerComponent()
        {
            m_Inputs = new List<InputComponent>();
        }

        public void AddInputComponent(InputComponent input)
        {
            m_Inputs.Add(input);
            var rcBounds = Bounds;

            rcBounds.Height = (m_Inputs.Count * 30) + 10;
            Bounds = rcBounds;            
        }

        public InputComponent getInput(int index)
        {
            return m_Inputs[index];
        }

        public override void OnPaint(System.Windows.Forms.PaintEventArgs e)
        {
            var g = e.Graphics;

            var sm = g.SmoothingMode;
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;

            var roundRect = GraphicsComponent.CreateRoundRect(Bounds.X, Bounds.Y, Bounds.Width, Bounds.Height, 10);

			var brush = new SolidBrush(Color.FromArgb(255, 63, 63, 63));
			var brLight = new LinearGradientBrush(Bounds, Color.FromArgb(100, Color.White), Color.FromArgb(100, Color.Black), 90, false);
			
            g.FillPath(brush, roundRect);
			g.FillPath(brLight, roundRect);

            g.SmoothingMode = sm;

            int yOutput = 10;
            foreach (var input in m_Inputs)
            {
                input.Bounds = new Rectangle(Bounds.Left - 10, Bounds.Y + yOutput, 20, 20);
                input.OnPaint(e);

                yOutput += 30;
            }
        }
    }
}
