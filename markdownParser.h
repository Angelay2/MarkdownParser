#include <iostream>
#include <string>
#include <vector>
#include <fstream>

/*
1. ͨ������markdown�﷨�����������е�ÿһ�����, ����Ҫת��Ϊhtml
2. ���ӽڵ���Ƕ�״���, �����������ĵ��Ŀ�ʼ

��չ:
�����װ
�����Ƕ���﷨����: ����
����,б��Ƕ���﷨����
����,���±�,ɾ����,�»���...
*/
using namespace std;

//1. �ȶ���ö����
enum Token {
	nul = 0,
	paragraph = 1,
	href = 2,
	ul = 3,
	ol = 4,
	li = 5,
	em = 6,
	strong = 7,
	hr = 8,    // ˮƽ�ָ���
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
	u = 19,// �»���
	del = 20,// ɾ����
	ano = 21 // ע��
};
// HTMLǰ�ñ�ǩ
const std::string frontTag[] = {
	"", "<p>", "", "<ul>", "<ol>", "<li>", "<em>", "<strong>", "<hr color=#CCCCCC size=1 />",
	"", "<blockquote>", "<h1>", "<h2>", "<h3>", "<h4>", "<h5>", "<h6>",
	"<pre><code>", "<code>", "<u>", "<del>", "<!--" };
// HTML ���ñ�ǩ
const std::string backTag[] = {
	"", "</p>", "", "</ul>", "</ol>", "</li>", "</em>",
	"</strong>", "", "", "</blockquote>", "</h1>", "</h2>",
	"</h3>", "</h4>", "</h5>", "</h66>", "</code></pre>", "</code>", "</u>", "</del>", "-->" };

// ������������
struct Node{
	// �﷨����
	int _type;
	// ���ӽڵ�
	vector<Node*> _child;
	// ���ݣ�ͼƬ�����ӣ�����+��ַ����
	// elem[0]: ������ʾ������
	// elem[1]: ������ַ��·��
	string elem[2];

	// ���ڵ�
	Node(int type)
		:_type(type)
	{}
};

// ������
class markdownParser{
public:
	markdownParser(const string& filename);
	
	void transform();

	void dfs(Node* root);
	
	void insert(Node* curNode, const char* str);

	// ���������﷨, �Ƚ�������
	// ����ֵ: �﷨���� + �﷨��Ӧ���ݵ���ʼλ��
	pair<int, const char*> parserType(const char* str);

	// �ж��Ƿ�Ϊˮƽ�ָ��� ---(���ߴ���3)  �ڴ������Ŵ���,
	bool isCutLine(const char* str);

	const char* processStr(const char* str);
	
	string getContents() const;
	// ����html
	void generateHtml();
	// ����
	void destory(Node* root);
	// ����
	~markdownParser();

private:
	// �﷨�������
	Node* _root;
	// �ļ���
	string _filename;
	// ���html�ĵ�����
	string _contents;
};