using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;
using System.Windows;

namespace ssi
{
    public class StimuliXmlReader
    {

        XmlReader xml;
        RecordHandler handler;

        public StimuliXmlReader(RecordHandler handler, XmlReader xml)
        {
            this.xml = xml;
            this.handler = handler;
        }

        public IStimuliTrigger readTrigger()
        {                                    
            xml.ReadToFollowing("trigger");
            IStimuliTrigger slide = null;
            StimuliTriggerType id = (StimuliTriggerType)Enum.Parse(typeof(StimuliTriggerType), xml.GetAttribute("id"));            

            switch (id)
            {
                case StimuliTriggerType.Button:
                    slide = StimuliTriggerButton.Load(xml, handler.NextButton);
                    break;
                case StimuliTriggerType.ButtonX:
                    slide = StimuliTriggerButtonX.Load(xml, handler.NextButton);
                    break;
                case StimuliTriggerType.Event:
                    slide = StimuliTriggerEvent.Load(xml, handler.List);
                    break;
                case StimuliTriggerType.Timer:
                    slide = StimuliTriggerTimer.Load(xml);
                    break;
            }

            return slide;

        }

        public IStimuliSource readSource()
        {
            xml.ReadToFollowing("source");
            IStimuliSource source = null;
            StimuliSourceType id = (StimuliSourceType)Enum.Parse(typeof(StimuliSourceType), xml.GetAttribute("id"));
            
            switch (id)
            {
                case StimuliSourceType.Blank:
                    source = StimuliSourceBlank.Load (xml);
                    break;
                case StimuliSourceType.Code:
                    source = StimuliSourceCode.Load(xml);
                    break;
                case StimuliSourceType.Text:
                    source = StimuliSourceText.Load(xml);
                    break;
                case StimuliSourceType.URL:
                    source = StimuliSourceURL.Load(xml, handler);
                    break;
            }

            return source;
        }

        public StimuliSlide readSlide()
        {
            xml.ReadToFollowing("slide");

            string label = xml.GetAttribute("label");
            IStimuliSource source = readSource();
            IStimuliTrigger trigger = readTrigger();

            return new StimuliSlide(source, trigger, label);
        }

        public StimuliList readList(StimuliList list)
        {
            try
            {
                xml.ReadToFollowing("list");
                uint size = uint.Parse(xml.GetAttribute("size"));
                for (uint i = 0; i < size; i++)
                {
                    list.Add(readSlide());
                    string s = this.handler.project.StimuliDir;
                }
            }
            catch (Exception e)
            {
                MessageBox.Show("ERROR: " + e.ToString());
            }
            
            return list;
        }
    }

    public class StimuliXmlWriter
    {

        XmlWriter xml;        

        public StimuliXmlWriter(XmlWriter xml)
        {
            this.xml = xml;            
        }

        public void writeList(StimuliList list)
        {
            xml.WriteStartElement("list");
            
            xml.WriteStartAttribute("size");
            xml.WriteString(list.Count.ToString());
            xml.WriteEndAttribute();

            foreach (StimuliSlide slide in list)
            {                
                writeSlide(slide);             
            }

            xml.WriteEndElement();
        }

        public void writeSlide(StimuliSlide slide)
        {
            xml.WriteStartElement("slide");            
            xml.WriteStartAttribute("label");
            xml.WriteString(slide.Label);
            xml.WriteEndAttribute();
            writeSource(slide.Source);
            writeTrigger(slide.Trigger);
            xml.WriteEndElement();
        }

        public void writeTrigger(IStimuliTrigger trigger)
        {
            xml.WriteStartElement("trigger");
            xml.WriteStartAttribute("id");
            xml.WriteString(trigger.getType().ToString());
            xml.WriteEndAttribute();            
            trigger.save(xml);
            xml.WriteEndElement();
        }

        public void writeSource(IStimuliSource source)
        {
            xml.WriteStartElement("source");
            xml.WriteStartAttribute("id");
            xml.WriteString(source.Type.ToString());
            xml.WriteEndAttribute();
            source.save(xml);
            xml.WriteEndElement();
        }
    }
}
