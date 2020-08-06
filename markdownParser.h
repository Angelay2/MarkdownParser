#include <iostream>
#include <string>
#include <vector>

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
};
// HTML前置标签
const std::string frontTag[] = {
	"", "<p>", "", "<ul>", "<ol>", "<li>", "<em>", "<strong>", "<hr color=#CCCCCC size=1 />",
	"", "<blockquote>", "<h1>", "<h2>", "<h3>", "<h4>", "<h5>", "<h6>",
	"<pre><code>", "<code>" };
// HTML 后置标签
const std::string backTag[] = {
	"", "</p>", "", "</ul>", "</ol>", "</li>", "</em>",
	"</strong>", "", "", "</blockquote>", "</h1>", "</h2>",
	"</h3>", "</h4>", "</h5>", "</h66>", "</code></pre>", "</code>" };
 
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
	markdownParser(const string& filename)
		:_root(new Node(nul))
		, _filename(filename)
	{}
	void transform(){
		// 打开文件
		ifstream fin(_filename);
		if (!fin.is_open()){
			cout << "file: " << _filename << "open failed" << endl;
		}
		bool inblock = false;// 在代码块里
		// 读取内容
		string rowStr;
		while (!fin.eof()){
			getline(fin, rowStr);
		
			// 预处理：处理行首空格, 一般是头删，但头删复杂； 选取
			const char* start = processStr(rowStr.c_str());

			// 判断是否为空内容， 
			if (!inblock && start == nullptr)
				continue; // 若为空， 直接处理下一行

			// 判断是否为水平分割线， 如果是，则创建水平分割线结点
			if (!inblock && isCutLine(start)){
				_root->_child.push_back(new Node(hr));
				continue;
			}
			// 语法解析 根据内容解析类型,拿到当前字符串遇到的第一个语法,再拿到语法标签后的内容
			// 可以解析的是行首的语法, 如果不是,则需要边插入边解析
			pair<int, const char*> typeRet = parserType(start); // 解析之后的类型的结果


			// 创建语法结点, 
			// 判断代码块结点
			if (typeRet.first == blockcode){
				// 判断是代码块的起始还是结束
				if (!inblock){// 之前在代码块外, 
					//则现在为代码块的起始,  则创建代码块结点
					_root->_child.push_back(new Node(blockcode));
				}
				// 如果之前是在代码块里, 则现在在代码块结束位置, 不需要创建新的代码块结点
				inblock = !inblock; 
				continue;
				// 内容还没放进去
			}

			// 判断是否为代码块中的代码
			if (inblock){// 如果是代码块中的
				_root->_child.back()->elem[0] += rowStr;// 按纯文本对待. 加整个字符串,读到什么就是什么
				_root->_child.back()->elem[0] += '\n'; // 多行加'\n'
				continue;// 继续处理下一行的内容
			}

			// 段落结点
			if (typeRet.first == paragraph){
				// 创建一个段落结点放到孩子的最后一个位置, 逐字符插入
				_root->_child.push_back(new Node(paragraph));
				insert(_root->_child.back(), typeRet.second);
				continue;
			}

			// 如果是标题
			if (typeRet.first >= h1 && typeRet.first <= 6){
				// 创建标题结点
				_root->_child.push_back(new Node(typeRet.first)); // 是几级就创建几级
				// 插入标题内容
				_root->_child.back()->elem[0] = typeRet.second;
				continue;
			}

			// 无序列表 真正内容是放在列表结点中的, 把无序列表的每一项放在一个大标签里,
			// 先判断最后一个孩子是否是无序列表, 如果是, 则当前就是无序列表的项
			if (typeRet.first == ul){
				// 判断是否为无序列表的第一项
				// 文档的开始(为空)或者语法树中此时的最后一个结点不是无序结点(说明可以接下来插入的肯定是无序列表的第一个结点)
				if (_root->_child.empty() || _root->_child.back()->_type != ul){
					// 创建无序列表
					_root->_child.push_back(new Node(ul));// ul是无序列表
				}
				// 给无序列表加入列表子节点
				Node* UNode = _root->_child.back();
				UNode->_child.push_back(new Node(li));// li是列表
				// 给列表子节点进行内容的插入
				insert(UNode->_child.back(), typeRet.second);
			}

		}
		
		cout << "transfrom finished" << endl;

		

		// 展开语法树， 生产html文档
	}
	// 逐字符进行内容插入
	void insert(Node* curNode, const char* str){}

	// 解析行首语法, 先解析标题
	// 返回值: 语法类型 + 语法对应内容的起始位置
	pair<int, const char*> parserType(const char* str){
		// 解析标题: # + 空格
		const char* ptr = str;
		int titleNum = 0;// 标题级数
		while (*ptr && *ptr == '#'){
			// 若头部是以#开始, 则是标题, 统计是几级标题
			++ptr;
			++titleNum;
		}
		if (*ptr == ' ' && titleNum > 0 && titleNum < 6){
			// 遇到空格, 则就是一个标题, 则创建返回类型,以及空格之后的内容(空格+1)
			return make_pair(h1 + titleNum - 1, ptr + 1); // h1(标题1)本身就是11
		}
		// 不是标题, 需重新解析, 把指针归位 = 起始位置
		ptr = str;

		// 2. 看是否是代码块 '''代码内容'''
		if (strncmp(ptr, "'''", 3) == 0){
			return make_pair(blockcode, ptr + 3);
		}
		// 3. 无序列表, 判断前两字符是否是横杠+空格(- )
		if (strncmp(ptr, "- ", 2) == 0){
			return make_pair(ul, ptr + 2);
		}
		// 4. 有序列表, 数字字符 + '.' + '空格'
		if (*ptr >= '0' && *ptr <= '9'){
			// 先把数字全部遍历完
			while (*ptr && *ptr >= '0' && *ptr <= '9'){// 存在且在0~9之间
				++ptr;
			}
			if (*ptr && *ptr == '.'){
				++ptr;
				if (*ptr == ' '){
					// 说明这是一个有序列表了
					return make_pair(ol, ptr + 1);// 这里已经走到空格了, 只需加1就到我们真正的内容了
				}
			}
			// 不符合要求, 则重置
			ptr = str;
		}

		// 5. 引用, ">" + 空格
		if (strncmp(ptr, "> ", 2) == 0){
			return make_pair(quote, ptr + 2);
		}
		// 其他语法 统一解析为段落结点
		return make_pair(paragraph, ptr);
	}

	// 判断是否为水平分割线 ---(横线大于3)  在代码块外才处理,
	bool isCutLine(const char* str){
		int cnt = 0;
		while (*str && *str == '-'){
			++str;
			++cnt;
		}
		return cnt >= 3;
	}

	const char* processStr(const char* str){
		while (*str){
			if (*str == ' ' || *str == '\t')// 有空格或tab键 往后走
				++str;
			else 
				break; // 否则停掉
		}
		if (*str == '\0')
			return nullptr; // 走到末尾
		return str;
	}

private:
	// 语法树根结点
	Node* _root;
	// 文件名
	string _filename;
	// 存放html文档内容
	string _contents;
};