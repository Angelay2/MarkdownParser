#include <iostream>
#include <string>
#include <vector>
#include <fstream>

/*
1. 通过解析markdown语法来创建树当中的每一个结点, 最终要转换为html
2. 孩子节点做嵌套处理, 根代表整个文档的开始

扩展:
代码封装
标题的嵌套语法解析: 链接
粗体,斜体嵌套语法解析
高亮,上下标,删除线,下划线...
*/
using namespace std;

//1. 先定义枚举类
enum Token {
	nul = 0,
	paragraph = 1,
	href = 2,
	ul = 3,
	ol = 4,
	li = 5,
	em = 6,
	strong = 7,
	hr = 8,    // 水平分割线
	image = 9,
	quote = 10,
	h1 = 11,
	h2 = 12,
	h3 = 13,
	h4 = 14,
	h5 = 15,
	h6 = 16,
	blockcode = 17,
	code = 18,
	u = 19,// 下划线
	del = 20,// 删除线
	ano = 21 // 注释
};
// HTML前置标签
const std::string frontTag[] = {
	"", "<p>", "", "<ul>", "<ol>", "<li>", "<em>", "<strong>", "<hr color=#CCCCCC size=1 />",
	"", "<blockquote>", "<h1>", "<h2>", "<h3>", "<h4>", "<h5>", "<h6>",
	"<pre><code>", "<code>", "<u>", "<del>", "<!--" };
// HTML 后置标签
const std::string backTag[] = {
	"", "</p>", "", "</ul>", "</ol>", "</li>", "</em>",
	"</strong>", "", "", "</blockquote>", "</h1>", "</h2>",
	"</h3>", "</h4>", "</h5>", "</h66>", "</code></pre>", "</code>", "</u>", "</del>", "-->" };

// 保存正文内容
struct Node{
	// 语法类型
	int _type;
	// 孩子节点
	vector<Node*> _child;
	// 内容（图片、链接（名称+地址））
	// elem[0]: 保存显示的内容
	// elem[1]: 保存网址、路径
	string elem[2];

	// 树节点
	Node(int type)
		:_type(type)
	{}
};

// 构建树
class markdownParser{
public:
	markdownParser(const string& filename);
	
	void transform();

	void dfs(Node* root);
	
	void insert(Node* curNode, const char* str);

	// 解析行首语法, 先解析标题
	// 返回值: 语法类型 + 语法对应内容的起始位置
	pair<int, const char*> parserType(const char* str);

	// 判断是否为水平分割线 ---(横线大于3)  在代码块外才处理,
	bool isCutLine(const char* str);

	const char* processStr(const char* str);
	
	string getContents() const;
	// 生成html
	void generateHtml();
	// 销毁
	void destory(Node* root);
	// 析构
	~markdownParser();

private:
	// 语法树根结点
	Node* _root;
	// 文件名
	string _filename;
	// 存放html文档内容
	string _contents;
};