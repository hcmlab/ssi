using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace FusionUI
{
    public partial class GridRangeSettingForm : Form
    {
        public RectangleF _ranges;
        public bool _autoSizeAvailable;
        public AutoSizeMode _autoSizeMode;
        public GridRangeSettingForm(RectangleF ranges, bool autoSizeAvailable, AutoSizeMode autoSizeMode)
        {
            InitializeComponent();
            _ranges = ranges;
            _autoSizeAvailable = autoSizeAvailable;
            _autoSizeMode = autoSizeMode;
        }

        private void GridRangeSettingForm_Load(object sender, EventArgs e)
        {
            numericUpDown1.Value = (decimal)_ranges.X;
            numericUpDown2.Value = (decimal)_ranges.Y;
            numericUpDown3.Value = (decimal)_ranges.Right;
            numericUpDown4.Value = (decimal)_ranges.Bottom;
            if (_autoSizeAvailable == false)
            {
                radioButton1.Checked = true;
            }
            else
            {
                if (_autoSizeMode == AutoSizeMode.GrowOnly)
                {
                    radioButton2.Checked = true;
                }
                else
                {
                    radioButton1.Checked = true;
                }
            }
        }

        private void radioButton1_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton1.Checked == true)
            {
                _autoSizeAvailable = false;
            }
        }

        private void radioButton2_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton2.Checked == true)
            {
                _autoSizeAvailable = true;
                _autoSizeMode = AutoSizeMode.GrowOnly;
            }
        }

        private void radioButton3_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton3.Checked == true)
            {
                _autoSizeAvailable = true;
                _autoSizeMode = AutoSizeMode.GrowAndShrink;
            }
        }
    }
}
