#ifndef __Mediator_User_Inc_H__
#define __Mediator_User_Inc_H__

#include "../stdafx.h"

namespace Mediator {

class Widget;

class DialogDirector {
public:
  virtual ~DialogDirector() { }
  CLASS_PTR(DialogDirector);

  virtual void ShowDialog() { CreateWidgets(); }
  virtual void WidgetChanged(std::shared_ptr<Widget> wiget) = 0;

protected:
  DialogDirector() { }
  virtual void CreateWidgets() = 0;
};

class Widget : public std::enable_shared_from_this<Widget> {
public:
  Widget(DialogDirector::Ptr director) {
    director_ = director;
  }
  virtual ~Widget() {}
  CLASS_PTR(Widget);

  virtual void Changed() {
    director_->WidgetChanged(shared_from_this());
  }
  virtual void HandleMouse() { }

protected:
  DialogDirector::Ptr director_;
};

class ListBox : public Widget {
public:
  ListBox(DialogDirector::Ptr director) : Widget(director) {
  }
  CLASS_PTR(ListBox);

  virtual const char* GetSelection() {
    return "test";
  }
  virtual void SetList(std::list<char*>& items) {
  }

  void HandleMouse() override {
    Changed();
  }
};

class Button : public Widget {
public:
  Button(DialogDirector::Ptr director) : Widget(director) {
  }
  CLASS_PTR(Button);

  virtual void SetText(std::string text) {
  }

  void HandleMouse() override {
    Changed();
  }
};

class FontDialogDirector
  : public DialogDirector,
    public std::enable_shared_from_this<FontDialogDirector> {
public:
  FontDialogDirector() { }
  ~FontDialogDirector() { }

  // 该函数的复杂度随对话框的增加而增加；在实践中，大的对话框并不受欢迎，其原因是
  // 多方面的，其中一个重要的原因是中介者的复杂性可能会抵消该模式在其他方面逮来的好处
  void WidgetChanged(Widget::Ptr widget) override {
    if (widget == ok_) {
      // TODO: ...
    } else if (widget == cancel_) {
      // TODO: ...
    } else if (widget == font_list_) {
      // TODO: ...
    }
  }

protected:
  void CreateWidgets() override {
    ok_ = std::make_shared<Button>(shared_from_this());
    cancel_ = std::make_shared<Button>(shared_from_this());
    font_list_ = std::make_shared<ListBox>(shared_from_this());
  }

private:
  Button::Ptr ok_;
  Button::Ptr cancel_;
  ListBox::Ptr font_list_;
};


void test() {
  LOG("\n\n mediator pattern.");

  //std::shared_ptr<DialogDirector> director = std::make_shared<FontDialogDirector>();
  auto director = std::make_shared<FontDialogDirector>();
  director->ShowDialog();

  LOG("director is drived by widgets.");
  LOG("if the widgets changed, the director will receive the signal.");
  // director is drived by widgets.
  // if the widgets changed, the director will receive the signal.
}

}

#endif //  __Mediator_User_Inc_H__
