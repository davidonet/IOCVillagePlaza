# Create your views here.
import os
from models import Topic
from models import Page
from models import Text
from django.conf import settings
from django.http import HttpResponse
from django.core.exceptions import ObjectDoesNotExist
from django.template import Context, loader

def text(request, topic, lang, page):
    try:
        text = Text.objects.get(lang=lang, page=Page.objects.get(pos=page, topic=Topic.objects.get(name=topic))).text
        text = text.replace('\\n', '\n'); 
    except ObjectDoesNotExist:
        text = "Not found"
    return HttpResponse(text, mimetype="text/plain")

def title(request, topic, lang, page):
    try:
        title = Text.objects.get(lang=lang, page=Page.objects.get(pos=page, topic=Topic.objects.get(name=topic))).title
        title = title.replace('\\n', '\n'); 
    except ObjectDoesNotExist:
        title = "Not found"
    return HttpResponse(title, mimetype="text/plain")

def imageurl(request, topic, lang, page):
    image = "http://" + request.get_host()
    image += "/media/" + topic + "/"   
    image += Page.objects.get(pos=page, topic=Topic.objects.get(name=topic)).image
    return HttpResponse(image, mimetype="text/plain")

def slideshow(request, topic, lang):
    t = loader.get_template('slideshow.html')
    c = Context({
                 'topic':topic,
                 'lang':lang,
                 'imgidx':[i+1 for i in range(len(Page.objects.filter(topic=Topic.objects.get(name=topic))))],
                 })
    return HttpResponse(t.render(c))

def index(request):
    t = loader.get_template('index.html')
    c = Context({
                'topics':Topic.objects.all()
    })
    return HttpResponse(t.render(c))


def image(request, topic, lang, page):
    import Image, ImageDraw, ImageFont, textwrap
    from django.utils.text import normalize_newlines
    from django.utils.safestring import mark_safe
       
    dir = settings.MEDIA_ROOT+'/render/'+lang+'/'+topic;
     
    if os.path.exists(dir+'/'+page+'.jpg'):
        return HttpResponse(open(dir+'/'+page+'.jpg'),mimetype="image/jpeg")
       
    ''' A View that Returns a PNG Image generated using PIL'''
  
    image = Image.open(Page.objects.get(pos=page, topic=Topic.objects.get(name=topic)).image)
    title = Text.objects.get(lang=lang, page=Page.objects.get(pos=page, topic=Topic.objects.get(name=topic))).title
    text = Text.objects.get(lang=lang, page=Page.objects.get(pos=page, topic=Topic.objects.get(name=topic))).text
    im = Image.new('RGBA', (1280, 800), (255, 255, 255, 255))
    draw = ImageDraw.Draw(im)
    im.paste(image, (0, 0))
    textFont = ImageFont.truetype("/home/dolivari/IOCVillagePlaza/IOCTabletServer/fonts/HelveticaLTStd-Roman.otf", 22)
    if len(title) < 32:
        titleFont = ImageFont.truetype("/home/dolivari/IOCVillagePlaza/IOCTabletServer/fonts/HelveticaNeueLTCom-BlkCn.ttf", 40)
    else:
        titleFont = ImageFont.truetype("/home/dolivari/IOCVillagePlaza/IOCTabletServer/fonts/HelveticaNeueLTCom-BlkCn.ttf", 34)
    width, height = titleFont.getsize(title)
    draw.text((1260 - width, 50), title, font=titleFont, fill=(32, 32, 32, 255))
    text = mark_safe(normalize_newlines(text))
    paragraphs = text.split('\n')
    width, height = textFont.getsize(text)
    y_text = 450 - (len(text) / 58) * height   
    for par in paragraphs:
        if(len(par) == 0):
            y_text += height
        lines = textwrap.wrap(par, width=54)
        for line in lines:
            width, height = textFont.getsize(line)
            draw.text((705, y_text), line, font=textFont, fill=(32, 32, 32, 255))
            y_text += height
    
    del draw # I'm done drawing so I don't need this anymore
    # We need an HttpResponse object with the correct mimetype
    response = HttpResponse(mimetype="image/jpeg")
    # now, we tell the image to save as a PNG to the 
    # provided file-like object
    im.save(response, 'jpeg', quality=90)
    if not os.path.exists(dir):
        os.makedirs(dir)
    im.save(open(dir+'/'+page+'.jpg','w+'),'jpeg', quality=90)

    return response # and we're done!

def img(request, topic, lang, page):
    import Image, ImageDraw, ImageFont, textwrap
    from django.utils.text import normalize_newlines
    from django.utils.safestring import mark_safe
       
    dir = settings.MEDIA_ROOT+'/render/'+lang+'/'+topic;
     
    if os.path.exists(dir+'/'+page+'_s.jpg'):
        return HttpResponse(open(dir+'/'+page+'_s.jpg'),mimetype="image/jpeg")
       
    ''' A View that Returns a PNG Image generated using PIL'''
  
    image = Image.open(Page.objects.get(pos=page, topic=Topic.objects.get(name=topic)).image)
    title = Text.objects.get(lang=lang, page=Page.objects.get(pos=page, topic=Topic.objects.get(name=topic))).title
    text = Text.objects.get(lang=lang, page=Page.objects.get(pos=page, topic=Topic.objects.get(name=topic))).text
    im = Image.new('RGBA', (1280, 800), (255, 255, 255, 255))
    draw = ImageDraw.Draw(im)
    im.paste(image, (0, 0))
    textFont = ImageFont.truetype("/home/dolivari/IOCVillagePlaza/IOCTabletServer/fonts/HelveticaLTStd-Roman.otf", 22)
    if len(title) < 32:
        titleFont = ImageFont.truetype("/home/dolivari/IOCVillagePlaza/IOCTabletServer/fonts/HelveticaNeueLTCom-BlkCn.ttf", 40)
    else:
        titleFont = ImageFont.truetype("/home/dolivari/IOCVillagePlaza/IOCTabletServer/fonts/HelveticaNeueLTCom-BlkCn.ttf", 34)
    width, height = titleFont.getsize(title)
    draw.text((1260 - width, 50), title, font=titleFont, fill=(32, 32, 32, 255))
    text = mark_safe(normalize_newlines(text))
    paragraphs = text.split('\n')
    width, height = textFont.getsize(text)
    y_text = 450 - (len(text) / 58) * height   
    for par in paragraphs:
        if(len(par) == 0):
            y_text += height
        lines = textwrap.wrap(par, width=54)
        for line in lines:
            width, height = textFont.getsize(line)
            draw.text((705, y_text), line, font=textFont, fill=(32, 32, 32, 255))
            y_text += height
    
    del draw # I'm done drawing so I don't need this anymore
    # We need an HttpResponse object with the correct mimetype
    response = HttpResponse(mimetype="image/jpeg")
    # now, we tell the image to save as a PNG to the 
    # provided file-like object
    im.thumbnail((640,400), Image.ANTIALIAS)
    im.save(response, 'jpeg', quality=90)
    if not os.path.exists(dir):
        os.makedirs(dir)
    im.save(open(dir+'/'+page+'_s.jpg','w+'),'jpeg', quality=90)
    return response # and we're done!



    