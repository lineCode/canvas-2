#ifndef _CANVAS_H_
#define _CANVAS_H_

#include <string>
#include <memory>

#include "Color.h"
#include "Surface.h"
#include "Image.h"

namespace canvas {
  class SavedContext {
  public:
    friend class Context;
    SavedContext() { }
  private:
    float globalAlpha = 0;
    bool imageSmoothingEnabled = false;
    float shadowBlur = 0;
    Color shadowColor;
    float shadowOffsetX = 0, shadowOffsetY = 0;
    Path current_path;
  };
  
  class Context {
  public:
    Context(float _display_scale = 1.0f)
      : display_scale(_display_scale) { }
    Context(const Context & other) = delete;
    Context & operator=(const Context & other) = delete;
    virtual ~Context() { }

    virtual std::shared_ptr<Surface> createSurface(const Image & image) = 0;
    virtual std::shared_ptr<Surface> createSurface(unsigned int _width, unsigned int _height, bool has_alpha) = 0;
    virtual std::shared_ptr<Surface> createSurface(const std::string & filename) = 0;

    virtual void resize(unsigned int _width, unsigned int _height);
        
    void beginPath() { current_path.clear(); }
    void closePath() { current_path.close(); }

    void arc(double x, double y, double r, double a0, double a1, bool t = false) { current_path.arc(x, y, r, a0, a1, t); }
    void moveTo(double x, double y) { current_path.moveTo(x, y); }
    void lineTo(double x, double y) { current_path.lineTo(x, y); }
    void arcTo(double x1, double y1, double x2, double y2, double radius) { current_path.arcTo(x1, y1, x2, y2, radius); }

    void clip() {
      getDefaultSurface().clip(current_path, getDisplayScale());
      current_path.clear();
    }
    void stroke() { renderPath(STROKE, strokeStyle); }
    void fill() { renderPath(FILL, fillStyle); }

    void save();
    void restore();

    TextMetrics measureText(const std::string & text) {
      return getDefaultSurface().measureText(font, text, getDisplayScale());
    }
    
    void rect(double x, double y, double w, double h);
    void fillRect(double x, double y, double w, double h);
    void strokeRect(double x, double y, double w, double h);
    void clearRect(double x, double y, double w, double h);

    void fillText(const std::string & text, double x, double y) { renderText(FILL, fillStyle, text, x, y); }
    void strokeText(const std::string & text, double x, double y) { renderText(STROKE, strokeStyle, text, x, y); }
    
    virtual Surface & getDefaultSurface() = 0;
    virtual const Surface & getDefaultSurface() const = 0;

    unsigned int getWidth() const { return getDefaultSurface().getLogicalWidth(); }
    unsigned int getHeight() const { return getDefaultSurface().getLogicalHeight(); }
    unsigned int getActualWidth() const { return getDefaultSurface().getActualWidth(); }
    unsigned int getActualHeight() const { return getDefaultSurface().getActualHeight(); }

    void drawImage(Context & other, double x, double y, double w, double h) {
      drawImage(other.getDefaultSurface(), x, y, w, h);
    }
    virtual void drawImage(const Image & img, double x, double y, double w, double h) {
      if (img.getData()) {
	auto surface = createSurface(img);
	drawImage(*surface, x, y, w, h);
      }
    }
    virtual void drawImage(Surface & img, double x, double y, double w, double h);
        
    Style & createLinearGradient(double x0, double y0, double x1, double y1) {
      current_linear_gradient.setType(Style::LINEAR_GRADIENT);
      current_linear_gradient.setVector(x0, y0, x1, y1);
      return current_linear_gradient;
    }

#if 0
    Style & createPattern(const Image & image, const char * repeat) {
      
    }
    Style & createRadialGradient(double x0, double y0, double r0, double x1, double y1, double r1) {
     }
#endif

    float lineWidth = 1.0f;
    Style fillStyle;
    Style strokeStyle;
    float shadowBlur = 0.0f;
    Color shadowColor;
    float shadowOffsetX = 0.0f, shadowOffsetY = 0.0f;
    float globalAlpha = 1.0f;
    Font font;
    TextBaseline textBaseline;
    TextAlign textAlign;
    bool imageSmoothingEnabled = true;
    
  protected:
    virtual void renderPath(RenderMode mode, const Style & style, Operator op = SOURCE_OVER);
    virtual void renderText(RenderMode mode, const Style & style, const std::string & text, double x, double y);

    bool hasShadow() const { return shadowBlur > 0 || shadowOffsetX != 0 || shadowOffsetY != 0; }
    float getDisplayScale() const { return display_scale; }
    
    Path current_path;

  private:
    // unsigned int width, height;
    Style current_linear_gradient;
    std::vector<SavedContext> restore_stack;
    float display_scale;
  };
  
  class FilenameConverter {
  public:
    FilenameConverter() { }
    virtual ~FilenameConverter() { }
    virtual bool convert(const std::string & input, std::string & output) = 0;
  };
  
  class NullConverter {
  public:
    bool convert(const std::string & input, std::string & output) { output = input; return true; }
  };
  
  class ContextFactory {
  public:
    ContextFactory(float _display_scale = 1.0f) : display_scale(_display_scale) { }
    virtual ~ContextFactory() { }
    virtual std::shared_ptr<Context> createContext(unsigned int width, unsigned int height, bool apply_scaling = true) = 0;
    virtual std::shared_ptr<Surface> createSurface(const std::string & filename) = 0;
    virtual std::shared_ptr<Surface> createSurface(unsigned int width, unsigned int height, bool has_alpha) = 0;
    virtual std::shared_ptr<Surface> createSurface(const unsigned char * buffer, size_t size) = 0;
    
    float getDisplayScale() const { return display_scale; }
    
  private:
    float display_scale;
  };
};

#endif
