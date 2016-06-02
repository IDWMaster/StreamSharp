
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
namespace StreamingVideo
{
    public abstract class VideoSource
    {
        
    }
	public class DisplaySource : VideoSource
	{
        [DllImport("../../../x64/Debug/DXAPI.dll")]
        static extern bool InitCapture_Screen();
		public DisplaySource()
		{
            InitCapture_Screen();
		}
	}
    public class VideoEncoder
    {
        VideoSource input;
        public VideoEncoder(VideoSource input)
        {
            this.input = input;
        }
    }
    public class VideoDecoder
    {

    }
}
