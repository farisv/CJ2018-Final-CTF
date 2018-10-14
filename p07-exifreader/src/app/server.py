import tornado.ioloop
import tornado.template
import tornado.web

import random
import string
import uuid
from PIL import ExifTags
from PIL import Image
from PIL import ImageFile
from os import path

class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.render("template/main.html")

class UploadHandler(tornado.web.RequestHandler):
    def post(self):
        file = self.request.files['file'][0]
        original_fname = file['filename']
        extension = path.splitext(original_fname)[1]

        allowed = ['.jpg', '.jpeg']

        if (extension not in allowed):
            self.finish('Invalid image')
            return

        fname = ''.join(random.choice(string.ascii_lowercase + string.digits) for x in range(16))
        final_filename= fname + extension

        img_path = 'temp/' + final_filename

        output_file = open(img_path, 'w')
        output_file.write(file['body'])
        output_file.close()

        img = Image.open(img_path)

        metadata = "No EXIF found"

        if (hasattr(img, '_getexif') and img._getexif()):
            exif = {
                ExifTags.TAGS[k]: v
                for k, v in img._getexif().items()
                if k in ExifTags.TAGS
            }
            metadata = ""
            for key, value in exif.iteritems():
                metadata += "<b>" + str(key) + "</b> : " + str(value) + "<br>"
            metadata += "<br><br>"

        w, h = img.size
        ratio = 350.0 / max(w, h)
        resized_image = img.resize((int(w * ratio), int(h * ratio)))
        resized_image.save(img_path)

        page = open('template/show.html', 'r').read()
        page = page.replace('METADATA_PLACEHOLDER', metadata)
        t = tornado.template.Template(page)
        self.write(t.generate(image_url=img_path))

class My404Handler(tornado.web.RequestHandler):
    def prepare(self):
        self.set_status(404)
        self.write('Page Not Found')

application = tornado.web.Application([
    (r"/", MainHandler),
    (r"/upload", UploadHandler),
    (r'/static/(.*)', tornado.web.StaticFileHandler, {'path': 'static'}),
    (r'/temp/(.*)', tornado.web.StaticFileHandler, {'path': 'temp'}),
], default_handler_class=My404Handler, debug=False)

if __name__ == '__main__':
    application.listen(8000)
    tornado.ioloop.IOLoop.instance().start()
