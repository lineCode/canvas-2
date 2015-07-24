#include "ContextQuartz2D.h"

#include <iostream>

using namespace canvas;
using namespace std;

Quartz2DSurface::Quartz2DSurface(const std::string & filename) : Surface(0, 0) {
  cerr << "trying to load file " << filename << endl;
  CGDataProviderRef provider = CGDataProviderCreateWithFilename(filename.c_str());
  CGImageRef img;
  if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".png") == 0) {
    img = CGImageCreateWithPNGDataProvider(provider, 0, false, kCGRenderingIntentDefault);
  } else if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".jpg") == 0) {
    img = CGImageCreateWithJPEGDataProvider(provider, 0, false, kCGRenderingIntentDefault);
  } else {
    cerr << "could not open file " << filename << endl;
    assert(0);
    img = 0;
  }
  CGDataProviderRelease(provider);
  colorspace = CGColorSpaceCreateDeviceRGB();
  if (img) {
    Surface::resize(CGImageGetWidth(img), CGImageGetHeight(img));
  
    bool has_alpha = CGImageGetAlphaInfo(img) != kCGImageAlphaNone;
    unsigned int bitmapBytesPerRow = getWidth() * 4; // (has_alpha ? 4 : 3);
    unsigned int bitmapByteCount = bitmapBytesPerRow * getHeight();
    bitmapData = new unsigned char[bitmapByteCount];
    memset(bitmapData, 0, bitmapByteCount);
  
    gc = CGBitmapContextCreate(bitmapData,
                               getWidth(),
                               getHeight(),
                               8,
                               bitmapBytesPerRow,
                               colorspace,
                               (has_alpha ? kCGImageAlphaPremultipliedLast : kCGImageAlphaNoneSkipLast)); // | kCGBitmapByteOrder32Big);
    initialize();
    CGContextDrawImage(gc, CGRectMake(0, 0, getWidth(), getHeight()), img);
    CGImageRelease(img);
  } else {
    resize(16, 16);
    unsigned int bitmapBytesPerRow = getWidth() * 4;
    unsigned int bitmapByteCount = bitmapBytesPerRow * getHeight();
    bitmapData = new unsigned char[bitmapByteCount];
    memset(bitmapData, 0, bitmapByteCount);
    
    gc = CGBitmapContextCreate(bitmapData,
                               getWidth(),
                               getHeight(),
                               8,
                               bitmapBytesPerRow,
                               colorspace,
                               kCGImageAlphaPremultipliedLast); // | kCGBitmapByteOrder32Big);
    initialize();
  }
}

Quartz2DSurface::Quartz2DSurface(const unsigned char * buffer, size_t size) : Surface(0, 0) {
  CGDataProviderRef provider = CGDataProviderCreateWithData(0, buffer, size, 0);
  CGImageRef img;
  if (isPNG(buffer, size)) {
    img = CGImageCreateWithPNGDataProvider(provider, 0, false, kCGRenderingIntentDefault);
  } else if (isJPEG(buffer, size)) {
    img = CGImageCreateWithJPEGDataProvider(provider, 0, false, kCGRenderingIntentDefault);
  } else {
    cerr << "could not open" << endl;
    assert(0);
    img = 0;
  }
  CGDataProviderRelease(provider);
  colorspace = CGColorSpaceCreateDeviceRGB();
  if (img) {
    resize(CGImageGetWidth(img), CGImageGetHeight(img));
  
    bool has_alpha = CGImageGetAlphaInfo(img) != kCGImageAlphaNone;
    unsigned int bitmapBytesPerRow = getWidth() * 4; // (has_alpha ? 4 : 3);
    unsigned int bitmapByteCount = bitmapBytesPerRow * getHeight();
    bitmapData = new unsigned char[bitmapByteCount];
    memset(bitmapData, 0, bitmapByteCount);
  
    gc = CGBitmapContextCreate(bitmapData,
                               getWidth(),
                               getHeight(),
                               8,
                               bitmapBytesPerRow,
                               colorspace,
                               (has_alpha ? kCGImageAlphaPremultipliedLast : kCGImageAlphaNoneSkipLast)); // | kCGBitmapByteOrder32Big);
    initialize();
    CGContextDrawImage(gc, CGRectMake(0, 0, getWidth(), getHeight()), img);
    CGImageRelease(img);
  } else {
    resize(16, 16);
    unsigned int bitmapBytesPerRow = getWidth() * 4;
    unsigned int bitmapByteCount = bitmapBytesPerRow * getHeight();
    bitmapData = new unsigned char[bitmapByteCount];
    memset(bitmapData, 0, bitmapByteCount);
    
    gc = CGBitmapContextCreate(bitmapData,
                               getWidth(),
                               getHeight(),
                               8,
                               bitmapBytesPerRow,
                               colorspace,
                               kCGImageAlphaPremultipliedLast); // | kCGBitmapByteOrder32Big);
    initialize();
  }
}

void
Quartz2DSurface::sendPath(const Path & path) {
  CGContextBeginPath(gc);
  for (auto pc : path.getData()) {
    switch (pc.type) {
    case PathComponent::MOVE_TO: CGContextMoveToPoint(gc, pc.x0, pc.y0); break;
    case PathComponent::LINE_TO: CGContextAddLineToPoint(gc, pc.x0, pc.y0); break;
    case PathComponent::ARC: CGContextAddArc(gc, pc.x0, pc.y0, pc.radius, pc.sa, pc.ea, pc.anticlockwise); break;
    case PathComponent::CLOSE: CGContextClosePath(gc); break;
    }
  }
}

void
Quartz2DSurface::fill(const Path & path, const Style & style) {
  if (style.getType() == Style::LINEAR_GRADIENT) {
    const std::map<float, Color> & colors = style.getColors();
    if (!colors.empty()) {
      std::map<float, Color>::const_iterator it0 = colors.begin(), it1 = colors.end();
      it1--;
      const Color & c0 = it0->second, c1 = it1->second;
      
      CGGradientRef myGradient;
      size_t num_locations = 2;
      CGFloat locations[2] = { 0.0, 1.0 };
      CGFloat components[8] = {
        c0.red, c0.green, c0.blue, c0.alpha,
        c1.red, c1.green, c1.blue, c1.alpha
      };
      
      myGradient = CGGradientCreateWithColorComponents(colorspace, components, locations, num_locations);
      
      //       Gdiplus::LinearGradientBrush brush(Gdiplus::PointF(Gdiplus::REAL(style.x0), Gdiplus::REAL(style.y0)),
      // 					 Gdiplus::PointF(Gdiplus::REAL(style.x1), Gdiplus::REAL(style.y1)),
      // 					 toGDIColor(c0),
      // 					 toGDIColor(c1));
      
      save();
      clip(path);
      
      CGPoint myStartPoint, myEndPoint;
      myStartPoint.x = style.x0;
      myStartPoint.y = style.y0;
      myEndPoint.x = style.x1;
      myEndPoint.y = style.y1;
      CGContextDrawLinearGradient(gc, myGradient, myStartPoint, myEndPoint, 0);
      restore();
      
      CGGradientRelease(myGradient);
    }
  } else {
    sendPath(path);
    CGContextSetRGBFillColor(gc, style.color.red,
			     style.color.green,
			     style.color.blue,
			     style.color.alpha);
    CGContextFillPath(gc);
  }
}

void
Quartz2DSurface::stroke(const Path & path, const Style & style, double lineWidth) {
  sendPath(path);
  CGContextSetRGBStrokeColor(gc, style.color.red,
			   style.color.green,
			   style.color.blue,
			   style.color.alpha);
  // CGContextSetLineWidth(context, fillStyle.);
  CGContextStrokePath(gc);  
}
