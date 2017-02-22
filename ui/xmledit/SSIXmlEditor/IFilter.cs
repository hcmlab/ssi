using System;
namespace SSIXmlEditor
{
    public interface IFilter
    {
        bool Matches(string value);
    }
}
