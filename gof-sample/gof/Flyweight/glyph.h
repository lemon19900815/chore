#pragma once

#include <unordered_map>
#include <memory>

#include "../stdafx.h"

namespace Flyweigt {

class Window;
class GlyphContext;
class Font;

// flyweight模式一般和组合模式一起使用
class Glyph {
public:
    virtual ~Glyph() { }
    CLASS_PTR(Glyph);

    virtual void Draw(Window*, GlyphContext&);
    virtual void SetFont(Font*, GlyphContext&);
    virtual Font* GetFont(GlyphContext&);
    virtual void First(GlyphContext&);
    virtual void Next(GlyphContext&);
    virtual bool IsDone(GlyphContext&);
    virtual Glyph::Ptr Current(GlyphContext&);

    // Composite模式接口
    virtual void Insert(Glyph::Ptr, GlyphContext&);
    virtual void Remove(GlyphContext&);
};

class Character : public Glyph {
public:
    Character(char ch);
    CLASS_PTR(Character);

    void Draw(Window*, GlyphContext&) override;

private:
    char ch_;
};

class Row : public Glyph {
public:
    Row();
    CLASS_PTR(Row);

    void Draw(Window*, GlyphContext&) override;

    void Insert(Glyph::Ptr, GlyphContext&) override;
    void Remove(GlyphContext&) override;

private:
    std::list<Glyph::Ptr> glyphs_;
};

class Column : public Glyph {
public:
    Column();
    CLASS_PTR(Column);

    void Draw(Window*, GlyphContext&) override;

    void Insert(Glyph::Ptr, GlyphContext&) override;
    void Remove(GlyphContext&) override;

private:
    std::list<Glyph::Ptr> glyphs_;
};

class GlyphContext {
public:
    GlyphContext();
    virtual ~GlyphContext();

    virtual void Next(int32_t step = 1);
    virtual void Insert(int32_t quantity = 1);

    virtual Font* GetFont();
    virtual void SetFont(Font*, int32_t span = 1);

private:
    int32_t index_;
    std::unordered_map<int32_t, Font*> font_dict_;
};

class GlyphFactory {
public:
    GlyphFactory();
    virtual ~GlyphFactory();

    virtual Character::Ptr CreateCharacter(char ch) {
        if (!characters_[ch]) {
            characters_[ch] = std::make_shared<Character>(ch);
        }
        return characters_[ch];
    }

    virtual Row::Ptr CreateRow() {
        return std::make_shared<Row>();
    }
    virtual Column::Ptr CreateColumn() {
        return std::make_shared<Column>();
    }

private:
    // flyweight
    Character::Ptr characters_[128];
};

}
