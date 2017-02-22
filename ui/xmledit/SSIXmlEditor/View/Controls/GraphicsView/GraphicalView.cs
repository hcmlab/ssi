using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Drawing2D;

namespace SSIXmlEditor.Controls.GraphicsView
{
    public partial class GraphicaView : Panel
    {
        private List<GraphicsComponent> m_Items;

        public GraphicaView()
        {
            this.BackColor = Color.White;
            //this.AutoScroll = true;
            this.HScroll = true;
            this.VScroll = true;

            InitializeComponent();

            m_Items = new List<GraphicsComponent>();

			//var sc = new SensorComponent();
			//sc.Bounds = new Rectangle(10, 10, 100, 100);
			//sc.AddOutputComponent(new OutputComponent() { Text = "audio" });
			//sc.AddOutputComponent(new OutputComponent() { Text = "mono" });
			//sc.AddOutputComponent(new OutputComponent() { Text = "stereo" });

			//AddComponent(sc);

			//var transformer = new TransformerComponent();
			//transformer.Bounds = new Rectangle(200, 10, 100, 100);
			//transformer.AddOutputComponent(new OutputComponent() { Text = "spect" });
			//transformer.AddInputComponent(new InputComponent() { Text = "pin_audio" });
			//m_Items.Add(transformer);

			//var consumer = new ConsumerComponent();
			//consumer.Bounds = new Rectangle(400, 10, 100, 100);
			//consumer.AddInputComponent(new InputComponent() { Text = "spect" });

			//AddComponent(consumer);

			//var link = new LinkComponent();
			//link.Output = sc.getOutput(0);
			//link.Input = transformer.getInput(0);

			//AddComponent(link);

			//var link2 = new LinkComponent();
			//link2.Output = transformer.getOutput(0);
			//link2.Input = consumer.getInput(0);

			//AddComponent(link2);
        }

        internal void AddComponent(GraphicsComponent component)
        {
            m_Items.Add(component);
            
            var collision = m_Items.Where(l => m_Items.Where(r => !l.Equals(r) && l.Bounds.IntersectsWith(r.Bounds)).Count() > 0);
            
            if(collision.Count() == 2)
			{
				var item = collision.Last();
				var rcNewBounds = item.Bounds;
				rcNewBounds.Offset(0, collision.First().Bounds.Height + 10);
				item.Bounds = rcNewBounds;
			}
        }
        
        internal void PerformLinking()
        {
			var sensors = m_Items.Where(comp => comp is SensorComponent);
			if(sensors.Count() == 0)
				return;
			
			var links = new List<GraphicsComponent>();	
			
			foreach(SensorComponent sensor in sensors)
			{
				foreach(var output in sensor.Outputs)
				{
					//transformer input
					var transformers = m_Items.Where(comp => comp is TransformerComponent);
					if (transformers.Count() > 0)
					{
						foreach (TransformerComponent transformer in transformers)
						{
							foreach(InputComponent input in transformer.Inputs)
							{
								if(input.Text.Equals(output.Text))
								{
									var link = new LinkComponent() { Input = input, Output = output };
									links.Add(link);
								}
							}

							var cons = m_Items.Where(comp => comp is ConsumerComponent);
							foreach(ConsumerComponent consumer in cons)
							{
								foreach(OutputComponent outp in transformer.Outputs)
								{
									foreach(var consInput in consumer.Inputs)
									{
										if(outp.Text.Equals(consInput.Text))
										{
											var link = new LinkComponent();
											link.Input = consInput;
											link.Output = outp;
											
											links.Add(link);
										}
									}
								}
							}
						}
					}

					var consumers = m_Items.Where(comp => comp is ConsumerComponent);
					if (consumers.Count() > 0)
					{
						foreach (ConsumerComponent consumer in consumers)
						{
							foreach (InputComponent input in consumer.Inputs)
							{
								if (input.Text.Equals(output.Text))
								{
									var link = new LinkComponent() { Input = input, Output = output };
									links.Add(link);
								}
							}
						}
					}
				}
			}
			
			foreach(var item in m_Items.Where(c => !(c is InputComponent) || !(c is OutputComponent)))
			{
				foreach(var link in links)
				{
					var col = item.Bounds.IntersectsWith(link.Bounds);
				
					while(item.Bounds.IntersectsWith(link.Bounds))
					{
						var rcBounds = item.Bounds;
						rcBounds.Offset(0, 10);
						item.Bounds = rcBounds;
					}						   					
				}
			}
			
			m_Items.AddRange(links);
			
        }

        protected override void OnResize(EventArgs eventargs)
        {
            base.OnResize(eventargs);

			if(m_Items.Count == 0)
				return;

            var maxWidth = m_Items.Max(comp => comp.Bounds.Right);
            var maxHeight = m_Items.Max(comp => comp.Bounds.Bottom);

            this.AutoScrollMinSize = new Size(maxWidth, maxHeight);
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            e.Graphics.TranslateTransform(this.AutoScrollPosition.X, this.AutoScrollPosition.Y);

            foreach (var item in m_Items)
                item.OnPaint(e);
        }
    }
}
