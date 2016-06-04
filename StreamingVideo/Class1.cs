
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
        public delegate void BeginRead_Callback(IntPtr bytes, uint count, IntPtr callback, IntPtr state);
        public delegate void EndRead_Callback(IntPtr callback, IntPtr bytesWritten);
        Stream str;
        public Video(Stream str)
        {
            this.str = str;
        }
        [DllImport("../../../x64/Debug/DXAPI.dll")]
        static extern void PlaybackStream(BeginRead_Callback br, EndRead_Callback er);
        public void Play(IntPtr hwnd)
        {
            uint bytesRead = 0;
            BeginRead_Callback br = async (_bytes, count, callback, state) => {
                byte[] tempBuffer = new byte[count];
                bytesRead = (uint)await str.ReadAsync(tempBuffer, 0, tempBuffer.Length);
                Marshal.Copy(tempBuffer, 0, _bytes, (int)bytesRead);
                DisplaySource.CompleteIO(callback, state);
            };
            EndRead_Callback er = (callback, _bytesRead)=> {
                unsafe
                {
                    uint* processed = (uint*)_bytesRead;
                    *processed = bytesRead;
                }
            };
            
            PlaybackStream(br,er);
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
       internal static extern void CompleteIO(IntPtr callback, IntPtr state);
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
