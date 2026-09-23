---
name: maccura-ppt-html
description: Use when users request 迈克生物 or maccura branded HTML presentations, PPT模板A styling, offline training slides, 双击 HTML 演示, or slide PNG images for insertion into PowerPoint.
---

# Maccura PPT HTML

用随附的 PPT 模板 A 原始底图制作演示文稿。默认交付**自包含单文件 HTML**：直接双击、新版 Edge/Chrome、断网可翻页、全屏演示并导出当前页 1920×1080 PNG。保留品牌，不重新设计 Logo，不把启动服务器作为使用前提。

## 生成流程

1. 根据用户给定材料确定主题、受众、页数和大纲；已有信息不要重复询问。未指定时使用中文、16:9、封面→目录→内容→封底。仪器维护参数、安全步骤、医学结论必须有用户材料或获准来源；缺失则明确待确认，不编造。
2. 将下方 `SKILL_DIR` 替换成本技能所在绝对目录。先检查目标父目录；初始化到**不存在的新目录**，不要覆盖用户已有文件，也不要直接编辑技能内模板。
3. 阅读生成的 `index.html`、`style.css`，替换示例文案，按需复制或移除 `.slide` 和对应 tab。六页只是版式样例，**不是页数限制**；规范页无需保留在业务演示中。
4. 更新 `<title>`、页头、封面副标题、元信息、目录、章节编号、`data-name`/`data-note`、页码和导航标签。PNG 前缀自动取自 `<title>`。新页 ID 使用字母开头的字母/数字/连字符/下划线；每页一个同序 tab，同步 `id`、`aria-labelledby`、`aria-controls`、`data-slide` 和目录的 `?slide=`。脚本自动查询 DOM，**不用手写或修改 slide 数组**；导航 CSS 已支持可变页数。
5. 修改完成后重新打包；交付构建结果，不能只交付引用本地资源的 `index.html`。输出已存在时，先确认是本次生成的产物，再用 `--overwrite`，不覆盖源 HTML。

```bash
python "SKILL_DIR/scripts/deck.py" init "./培训课件" --title "仪器维护培训"
# 编辑培训课件中的 HTML/CSS，填写经确认的实际内容后执行
python "SKILL_DIR/scripts/deck.py" build "./培训课件" --output "./仪器维护培训-离线版.html"
node "SKILL_DIR/scripts/verify-offline.cjs" "./仪器维护培训-离线版.html"
```

构建需 Python 3.9+，自动验证需 Node.js 22+ 和 Edge/Chrome；均不需 npm/pip 安装。验证器第二参数可指定浏览器可执行文件；最终观看者只需要浏览器，不需要 Python、Node 或本技能。

## 品牌与排版速查

| 元素 | 保持的规范 |
|---|---|
| 画幅 | 16:9；`.slide` 等比缩放，百分比定位与 `cqw` 字号 |
| 配色 | 亮蓝 `#00B4FF`，正文 `#474D59`，中灰 `#AAAFB4`，浅灰 `#EBEEF2`，白 `#FFFFFF` |
| 字体 | 系统已安装的微软雅黑 / Microsoft YaHei 及 Light；**不打包字体文件**，无字体时说明替代显示差异 |
| 字号比例 | 封面/章节标题48pt→5cqw；章节副标题32pt→3.33cqw；内页标题36pt→3.75cqw；正文18pt→1.875cqw |
| 内页安全区 | 左右约5.12%；标题顶部14.6%；避开顶部品牌区；长内容拆页而非压缩正文 |
| 底图 | `cover` 封面、`catalogue` 目录、`chapter` 章节、`content` 内页、`closing` 封底 |

五张 PNG 来自用户提供的 PPT模板A.pptx 的 image1–image5，按上述顺序保存于 `assets/template/assets/`。底图内已有 Logo、版权、目录/CHAPTER/封底口号，不重复绘制或遮挡。仅在用户授权的品牌用途内使用，不自动发布或上传资源。中灰/亮蓝不要作为白底小号正文；保留深灰正文与充分留白。

## 全屏演示

- 保留「全屏演示」按钮及 `.fullscreen-toolbar`：点击按钮或按 F 进入，按 F / Esc 或「退出全屏」返回；← / → 翻页，全屏时 Home / End 跳转首尾页。
- 使用浏览器 Fullscreen API 让 `.preview` 整体全屏，不单独全屏某个 `.slide`。画面保持 16:9、等比居中，底部操作栏不得遮挡品牌或进入 PNG；全屏下也能导出当前页。
- 保留 `fullscreenchange` 状态同步、退出焦点恢复、重复切换保护及失败提示。浏览器拒绝全屏时仍可翻页和导出；不使用关闭安全策略的启动参数。应在独立 Edge/Chrome 中打开，不以 IDE 内嵌预览器作为全屏验收依据。

## 离线与导出约束

- 复用 `preview.js` 的导航、导出锁、失败恢复及渲染流程。按钮在 `.slide` 外，导出只包括当前幻灯片；不得改成整个浏览器截图。
- 品牌底图使用 `<img class="slide-background" data-background="content" ...>`；打包器内嵌五张原图并生成 `assets/backgrounds.js`。新增图示先转为 `data:image/...;base64,...`；不要引用相对 PNG、网络图片、CDN、在线字体或运行时 fetch 本地文件。
- 构建器仅处理模板的固定 CSS/JS 引用，保留这三个引用标记；新增代码写入现有文件，不增加外部脚本/样式链接。其静态检查不是任意 JavaScript 的安全或离线保证，仍须浏览器验证。
- 避免依赖伪元素、复杂 SVG、视频、嵌套页面、滤镜等未经验证的导出效果；优先真实 HTML 文本、简单 CSS 和内嵌位图。
- `SecurityError` / tainted canvas 表明资源未正确内嵌；修复数据来源，**不能通过关闭浏览器安全策略或要求 HTTP 打开来绕过**。

## 验收与交付

- 对最终单文件运行验证器：它仅复制 HTML 到独立临时目录，以标准浏览器 `file://` 打开并禁网，动态遍历全部页面，检查关联、底图、1920×1080、重复点击、失败后重试、实际 PNG 落盘和零 HTTP 请求；同时验证全屏进入/退出、F / Esc、翻页、窄窗口、拒绝后重试及普通/全屏导出回归。仅对本次生成的可信 HTML 执行工具，不运行来历不明的主动内容。
- 打开输出证据目录中的真实 PNG 和浏览器截图，逐页检查内容、图文重叠、品牌遮挡、裁切、文字可读性；检查全屏横屏/窄屏截图中画幅和操作栏互不遮挡，另测普通模式导航标签获焦时 Home/End 首尾页。自动测试通过不等于视觉验收通过。
- 无浏览器或未实际下载时，准确说明未测项，不能声称“离线导出已验证”。只看 localhost 页面、只生成 Blob 或只验第一张均不充分。
- 返回单文件路径、页数、实测浏览器和未测限制。说明 PPT 选择16:9宽屏，将 PNG 设置为幻灯片背景即可铺满；**PNG 中的文字不可单独编辑**，本技能不直接生成可编辑 PPTX。
