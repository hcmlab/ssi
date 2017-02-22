using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing;

namespace SSIXmlEditor.View
{
	class ColorTable : ProfessionalColorTable
	{
		public override System.Drawing.Color MenuStripGradientBegin
		{
			get
			{
				return Color.FromArgb(255, 63, 63, 63);
			}
		}

		public override Color MenuStripGradientEnd
		{
			get
			{
				return Color.FromArgb(255, 63, 63, 63);
			}
		}

		public override Color MenuItemSelectedGradientBegin
		{
			get
			{
				return Color.FromArgb(255, 192, 210, 238);
			}
		}

		public override Color MenuItemSelectedGradientEnd
		{
			get
			{
				return this.MenuItemSelectedGradientBegin;
			}
		}

		public override Color MenuItemBorder
		{
			get
			{
				return Color.White;
			}
		}

		public override Color MenuItemSelected
		{
			get
			{
				return this.MenuItemSelectedGradientBegin;
			}
		}

		public override Color MenuItemPressedGradientBegin
		{
			get
			{
				return Color.FromArgb(255, 65, 65, 65);
			}
		}
		
		public override Color ImageMarginGradientBegin
		{
			get
			{
				return Color.FromArgb(255, 63, 63, 63);
			}
		}

		public override Color ImageMarginGradientMiddle
		{
			get
			{
				return Color.FromArgb(255, 64, 64, 64);
			}
		}

		public override Color ImageMarginGradientEnd
		{
			get
			{
				return Color.FromArgb(255, 65, 65, 65);
			}
		}
	}
}
