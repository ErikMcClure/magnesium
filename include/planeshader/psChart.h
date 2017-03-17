// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __CHART_H__PS__
#define __CHART_H__PS__

#include "psTexFont.h"
#include "psSolid.h"
#include "psColored.h"
#include "bss-util\cStr.h"
#include "bss-util\cDynArray.h"

namespace planeshader {
  struct psChart : public psDriverHold
  {
  public:
    inline const psRect& GetExtrema() { return _extrema; }
    virtual void Render(const psRect& rect, const psRect& view) = 0;
    virtual bool SetCaption(size_t index, const char* caption) = 0;
    inline void SetFlags(psFlag flags) { _flags = flags; }
    inline psFlag GetFlags() const { return _flags; }
    virtual size_t GetLegend(psColor32* colors, const char** strings) = 0;
    inline virtual psChart* Clone() const = 0;

    enum CHART_FLAGS
    {
      CHART_DRAWVALUES = 0b1,
      CHART_DRAWLABELS = 0b10,
      CHART_DRAWLEGEND = 0b100
    };

  protected:
    psRect _extrema;
    bss_util::cDynArray<cStr, size_t, bss_util::CARRAY_SAFE> _captions;
    psFlag _flags;
  };

  // Renders a line graph of samples that must be sorted on the y-axis but not evenly spaced. You can render multiple line graphs on top of each other.
  class psLineChart : public psChart, public psColored
  {
  public:
    psLineChart(const psLineChart&);
    virtual void Render(const psRect& rect, const psRect& view);
    virtual bool SetCaption(size_t index, const char* caption);
    virtual size_t GetLegend(psColor32* colors, const char** strings);
    inline virtual psLineChart* Clone() const { return new psLineChart(*this); }

  protected:
    psColor32 _outline;
    psColor32 _fill;
    float _width;
    psTex* _data;
  };

  // Renders a bar graph of several variables. You can render multiple bar graphs on top of each other or next to each other.
  class psBarChart : public psChart, public psColored
  {
  public:
    psBarChart(const psBarChart&);
    virtual void Render(const psRect& rect, const psRect& view);
    virtual bool SetCaption(size_t index, const char* caption);
    virtual size_t GetLegend(psColor32* colors, const char** strings);
    inline virtual psBarChart* Clone() const { return new psBarChart(*this); }

  protected:
    psColor32 _fill;
    float _width;
    psTex* _data;
  };

  // Simple assigns a point to each element of the dataset, which can be unordered and take on arbitrary values.
  class psScatterChart : public psChart, public psColored
  {
  public:
    psScatterChart(const psScatterChart&);
    virtual void Render(const psRect& rect, const psRect& view);
    virtual bool SetCaption(size_t index, const char* caption);
    virtual size_t GetLegend(psColor32* colors, const char** strings);
    inline virtual psScatterChart* Clone() const { return new psScatterChart(*this); }

  protected:
    psColor32 _fill;
    void* _data;
  };

  // Renders a pie chart of several variables.
  class psPieChart : public psChart
  {
  public:
    virtual void Render(const psRect& rect, const psRect& view);
    virtual bool SetCaption(size_t index, const char* caption);
    virtual size_t GetLegend(psColor32* colors, const char** strings);
    inline virtual psPieChart* Clone() const { return new psPieChart(*this); }

  protected:
    psTex* _data;
  };

  // Acts as a container for multiple overlayed charts, and optionally renders a standard background.
  class psChartContainer : public psSolid, public psDriverHold, public psColored
  {
  public:
    psChartContainer(psTexFont* font = 0);
    inline void SetView(const psRect& view) { _view = view; }
    inline void SetViewRel(const psRect& view) { _viewrel = view; }
    inline void SetTitle(const char* title) { _title = title; }
    inline void SetXLabel(const char* label) { _xlabel = label; }
    inline void SetYLabel(const char* label) { _ylabel = label; }
    void SetDim(const psVec& dim);
    size_t AddChart(psChart* chart);
    psChart* GetChart(size_t index);
    bool RemoveChart(size_t index);

    enum CONTAINER_FLAGS
    {
      CONTAINER_HIDEX = 0b1,
      CONTAINER_HIDEY = 0b10,
      CONTAINER_INVERTX = 0b100,
      CONTAINER_INVERTY = 0b1000,
    };

  protected:
    virtual void _render(const psParent& parent) override;

    psTexFont* _font;
    cStr _title;
    cStr _xlabel;
    cStr _ylabel;
    psRect _view;
    psRect _viewrel;
    bss_util::cDynArray<std::unique_ptr<psChart>, size_t, bss_util::CARRAY_SAFE> _captions;
    psFlag _flags;
    psColor32 _textcolor;
  };
}

#endif