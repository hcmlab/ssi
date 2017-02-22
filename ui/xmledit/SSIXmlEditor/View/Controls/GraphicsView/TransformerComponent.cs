using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Drawing.Drawing2D;

namespace SSIXmlEditor.Controls.GraphicsView
{
    class TransformerComponent : GraphicsComponent
    {
        private List<InputComponent> m_Inputs;
        private List<OutputComponent> m_Outputs;

		public IEnumerable<InputComponent> Inputs { get { return m_Inputs; } }
		public IEnumerable<OutputComponent> Outputs { get { return m_Outputs; } }

        public TransformerComponent()
        {
            m_Inputs = new List<InputComponent>();
            m_Outputs = new List<OutputComponent>();
        }

        public void AddInputComponent(InputComponent input)
        {
            m_Inputs.Add(input);
            var rcBounds = Bounds;

            if (m_Outputs.Count < m_Inputs.Count)
            {
                rcBounds.Height = (m_Inputs.Count * 30) + 10;
                Bounds = rcBounds;
            }
        }

        public void AddOutputComponent(OutputComponent output)
        {
            m_Outputs.Add(output);

            var rcBounds = Bounds;

            if (m_Inputs.Count < m_Outputs.Count)
            {
                rcBounds.Height = (m_Outputs.Count * 30) + 10;
                Bounds = rcBounds;
            }
        }

        public InputComponent getInput(int index)
        {
            return m_Inputs[index];
        }

        public OutputComponent getOutput(int index)
        {
            return m_Outputs[index];
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
            foreach (var output in m_Outputs)
            {
                output.Bounds = new Rectangle(Bounds.Right - 10, Bounds.Y + yOutput, 20, 20);
                output.OnPaint(e);

                var rcText = output.Bounds;
                rcText.Inflate(40, 0);
                rcText.Offset(0, 0);

                g.DrawString(output.Text, SystemFonts.DefaultFont, Brushes.White, rcText);

                yOutput += 30;
            }

            yOutput = 10;
            foreach (var input in m_Inputs)
            {
                input.Bounds = new Rectangle(Bounds.Left - 10, Bounds.Y + yOutput, 20, 20);
                input.OnPaint(e);

                yOutput += 30;
            }
        }
    }
}
