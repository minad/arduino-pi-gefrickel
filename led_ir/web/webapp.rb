require 'sinatra'
require 'slim'
require 'fileutils'

set :public_folder, Proc.new { File.join(root, 'static') }

get '/' do
  slim :index
end

get '/color/:hex' do
  hex = [params[:hex]].pack('H*')
  if hex.size == 3
    begin
      s = TCPSocket.new 'localhost', 7777
      s.write('rgb' + hex)
    ensure
      s.close
    end
    params[:hex]
  else
    'error'
  end
end

get '/cam.jpg' do
  file = '/tmp/cam.jpg'
  tmpfile = '/tmp/camtmp.jpg'
  if !File.exists?(tmpfile) && (!File.exists?(file) || File.mtime(file) + 10 < Time.now)
    FileUtils.touch(tmpfile)
    Thread.new { `fswebcam #{tmpfile}`; File.rename(tmpfile, file) }
  end
  mime_type 'image/jpeg'
  send_file file
end

__END__

@@ layout
doctype html
html
  head
    link rel="stylesheet" href="css/colorpicker.css" type="text/css"/
    script type="text/javascript" src="js/jquery.js"
    script type="text/javascript" src="js/colorpicker.js"
  body
    == yield

@@ index
h1 Teilchens Beleuchtung
p#colorpicker
javascript:
  $(function() {
    var ready = true;
    $('#colorpicker').ColorPicker({flat: true,
                                 onChange: function(hsb, hex, rgb) {
                                    if (ready) {
                                        $.ajax('/color/' + hex).done(function() { ready = true; });
                                        ready = false;
                                    }
                                 }});
  });
  setInterval(function() {
          $('#cam').attr('src', '/cam.jpg?' + (new Date()).getTime());
  }, 5000);
img#cam src='/cam.jpg'

