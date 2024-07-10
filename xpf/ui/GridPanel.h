#pragma once
#include <xpf/ui/Panel.h>

namespace xpf {

class GridPanel;

typedef AttachedProperty<uint8_t, ConstexprHash("grid_column")> Grid_Column;
typedef AttachedProperty<uint8_t, ConstexprHash("grid_row")> Grid_Row;

class GridPanel : public Panel
{
public:
    std::vector<PanelLength> m_columns;
    std::vector<PanelLength> m_rows;
    std::vector<UIElement*> m_sortedChildren;
    std::vector<float> m_row_lengths;
    std::vector<float> m_col_lengths;

public:
    GridPanel()
        : Panel(UIElementType::GridPanel)
    {}

    GridPanel& AddColumns(std::vector<PanelLength>&& columns) { m_columns = std::move(columns); InvalidateLayout(); return *this; }
    GridPanel& AddColumn(const PanelLength& column) { m_columns.push_back(column); InvalidateLayout(); return *this; }
    GridPanel& AddRows(std::vector<PanelLength>&& rows) { m_rows = std::move(rows); InvalidateLayout(); return *this; }
    GridPanel& AddRow(const PanelLength& row) { m_rows.push_back(row); InvalidateLayout(); return *this; }

    // constraint is without margin, border or padding.
    // returns inner desired size, without margin, border or padding.
    virtual v2_t OnMeasure(v2_t constraint) override
    {
        if (m_columns.empty())
            m_columns = {Panel_OneStar};

        if (m_rows.empty())
            m_rows = {Panel_OneStar};

        v2_t gridDesiredSize = constraint;

        for (const auto& spChild : m_children)
        {
            v2_t childSize = spChild->Measure(constraint); // child size including its margin

            gridDesiredSize.w = std::max(gridDesiredSize.w, childSize.w);
            gridDesiredSize.h = std::max(gridDesiredSize.h, childSize.h);
        }

        m_row_lengths = MeasureColumnsRows(gridDesiredSize, /*rows*/true);
        m_col_lengths = MeasureColumnsRows(gridDesiredSize, /*rows*/false);

        m_sortedChildren.clear();
        m_sortedChildren.resize(m_row_lengths.size() * m_col_lengths.size());

        for (const auto& spChild : m_children)
        {
            uint8_t row = spChild->GetValueOr<Grid_Row>(0);
            uint8_t col = spChild->GetValueOr<Grid_Column>(0);
            v2_t childSize(m_col_lengths[col], m_row_lengths[row]);
            spChild->Measure(childSize);
            m_sortedChildren[m_col_lengths.size() * row + col] = spChild.get();
        }

        return gridDesiredSize;
    }

    // insideSize is without margin, border or padding, computed from m_insideRect.
    virtual void OnArrange(v2_t insideSize) override
    {
        size_t index = 0;
        float y = m_insideRect.y;
        for (size_t row = 0; row < m_row_lengths.size(); row++)
        {
            float x = m_insideRect.x;
            float rowHeight = m_row_lengths[row];
            for (size_t col = 0; col < m_col_lengths.size(); col++)
            {
                float columnWidth = m_col_lengths[col];
                UIElement* pChild = m_sortedChildren[index];
                if (pChild != nullptr)
                {
                    pChild->Arrange({x, y, columnWidth, rowHeight});
                }

                index++;
                x += columnWidth;
            }

            y += rowHeight;
        }
    }

   std::vector<float> MeasureColumnsRows(v2_t insideSize, bool rows)
   {
       float len_stars = 0;
       float len_pixels = 0;
       const std::vector<PanelLength>& lengths = rows ? m_rows : m_columns;
       float len_total = rows ? insideSize.y : insideSize.x;

       uint8_t index = 0;
       std::vector<float> widths = {0};
       widths.resize(lengths.size());

       for (const PanelLength& len : lengths)
       {
           if (len.type == PanelLength::Auto)
           {
               for (const auto& spChild : m_children)
               {
                   uint8_t num = rows ? spChild->GetValueOr<Grid_Row>(0) : spChild->GetValueOr<Grid_Column>(0);
                   if (num == index)
                   {
                        v2_t childSize = spChild->GetDesiredSize();
                        widths[index] = std::max(widths[index], (rows ? childSize.y : childSize.x));
                   }
               }

               len_pixels += widths[index];
           }
           else if (len.type == PanelLength::Star)
           {
               len_stars += len.length;
           }
           else
           {
               len_pixels += len.length;
           }

           index++;
       }

        float len_pixels_removed = (len_total - len_pixels);
        if (len_pixels_removed < 0)
            len_pixels_removed = len_total;

        float len_per_star = len_stars > 0 ? len_pixels_removed / len_stars : 0;

        float suggested_len = 0;

        index = 0;
        for (const PanelLength& len : lengths)
        {
            if (len.type == PanelLength::Auto)
            {
                // noop - we already computed above
            }
            else if (len.type == PanelLength::Star)
            {
                widths[index] = len.length * len_per_star;
            }
            else
            {
                widths[index] = len.length;
            }

            if (lengths[index].minLength > widths[index])
                widths[index] = lengths[index].minLength;
            if (lengths[index].maxLength < widths[index])
                widths[index] = lengths[index].maxLength;

            index++;
        }

        return widths;
    }

    virtual void OnDraw(IRenderer& r) override
    {
        Panel::OnDraw(r);
    }
};

} // xpf