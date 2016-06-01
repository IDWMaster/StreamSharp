
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StreamingVideo
{
    public abstract class VideoSource
    {
        
    }
	public class DisplaySource : VideoSource
	{
		public DisplaySource()
		{
		
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
