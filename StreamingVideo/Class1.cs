
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.IO;
namespace StreamingVideo
{
    public abstract class VideoSource
    {
        /// <summary>
        /// Renders a video to a Stream
        /// </summary>
        /// <param name="output"></param>
        public abstract void Record(Stream output);
    }
    public class Video
    {
        Stream str;
        public Video(Stream str)
        {
            this.str = str;
        }
        [DllImport("../../../x64/Debug/DXAPI.dll")]
        static extern void PlaybackStream();
        public void Play(IntPtr hwnd)
        {
            PlaybackStream();
        }
    }
	public class DisplaySource : VideoSource
	{
        public delegate void StreamCB(IntPtr ptr);
        
        public delegate void BeginWrite_Callback(IntPtr bytes, uint count, IntPtr callback, IntPtr state);
        public delegate void EndWrite_Callback(IntPtr callback, IntPtr bytesWritten);
        [DllImport("../../../x64/Debug/DXAPI.dll")]
        static extern bool InitCapture_Screen(StreamCB cb,BeginWrite_Callback BeginWrite, EndWrite_Callback EndWrite);
        [DllImport("../../../x64/Debug/DXAPI.dll")]
        static extern void CompleteIO(IntPtr callback, IntPtr state);
        public DisplaySource()
		{
		}
        public override void Record(Stream output)
        {
            IntPtr stream;
            uint written = 0;
            BeginWrite_Callback beginWrite = async (_bytes, count, callback, state) =>
            {
                byte[] bytes = new byte[count];
                Marshal.Copy(_bytes, bytes, 0, (int)count);
                await output.WriteAsync(bytes, 0, bytes.Length);
                written = count;
                CompleteIO(callback,state);
            };
            EndWrite_Callback endWrite = (callback, _bytesWritten) =>
            {
                unsafe
                {
                    uint* bwritten = (uint*)_bytesWritten;
                    *bwritten = written;
                }
            };
            InitCapture_Screen(m=> {
                stream = m;
            },beginWrite, endWrite);
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
