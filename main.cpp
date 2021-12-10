#include <bits/stdc++.h>
using namespace std;
const int maxn = 10;
struct Student
{
    string Num, Name, All;
    int Score[maxn];
    Student() : Num(""), Name(""), All("") { memset(Score, 0, sizeof(Score)); }
};
vector<Student> student;

void Add()
{
    string Num, a, b, Name;
    int score[maxn];
    cin >> Num >> score[0] >> score[1] >> score[2] >> a >> b;
    Name = a + " " + b;
    Student temp;
    temp.Num = Num;
    temp.Name = Name;
    temp.All = temp.Num + " " + temp.Name + " " + to_string(score[0]) + " " + to_string(score[1]) + " " + to_string(score[2]) + " ";
    for (int i = 0; i < 3; i++)
        temp.Score[i] = score[i];
    for (auto &i : student)
    {
        if (i.Num == temp.Num)
        {
            i = temp;
            return;
        }
    }
    student.push_back(temp);
    // cout<<temp.All<<endl;
}
void Modify()
{
    string Num, a, b, Name;
    int score[maxn];
    cin >> Num >> score[0] >> score[1] >> score[2] >> a >> b;
    Name = a + " " + b;
    Student temp;
    temp.Num = Num;
    temp.Name = Name;
    temp.All = temp.Num + " " + temp.Name + " " + to_string(score[0]) + " " + to_string(score[1]) + " " + to_string(score[2]) + " ";
    for (int i = 0; i < 3; i++)
        temp.Score[i] = score[i];
    for (int i = 0; i < student.size(); i++)
    {
        if (student[i].Num == temp.Num)
        {
            student[i] = temp;
        }
    }
}
void Delete()
{
    string Num;
    cin >> Num;
    for (int i = 0; i < student.size(); i++)
    {
        if (student[i].Num == Num)
        {
            student.erase(student.begin() + i);
        }
    }
}
void SearchNum()
{
    string Num;
    cin >> Num;
    for (int i = 0; i < student.size(); i++)
    {
        if (student[i].Num == Num)
        {
            cout << student[i].All << endl;
        }
    }
}
bool CompareNum(Student a, Student b)
{
    return a.Num < b.Num;
}
void SearchName()
{
    string a, b, Name;
    cin >> a >> b;
    Name = a + " " + b;
    vector<Student> temp;
    temp.clear();
    for (int i = 0; i < student.size(); i++)
    {
        if (student[i].Name == Name)
        {
            temp.push_back(student[i]);
        }
    }
    sort(temp.begin(), temp.end(), CompareNum);
    for (int i = 0; i < temp.size(); i++)
    {
        cout << temp[i].All << endl;
    }
}
void CompareNumMain()
{
    sort(student.begin(), student.end(), CompareNum);
    for (int i = 0; i < student.size(); i++)
        cout << student[i].All << endl;
}
bool CompareScore(Student a, Student b)
{
    int ax = a.Score[0] + a.Score[1] + a.Score[2];
    int bx = b.Score[0] + b.Score[1] + b.Score[2];
    return (ax > bx) || (ax == bx && a.Num < b.Num);
}
void CompareScoreMain()
{
    sort(student.begin(), student.end(), CompareScore);
    for (int i = 0; i < student.size(); i++)
        cout << student[i].All << endl;
}
void Quit()
{
    exit(0);
}
/*void Input() {
    int x; cin>>x;
    switch (x)
    {
        case 1: Add(); break;
        case 2: Modify(); break;
        case 3: Delete(); break;
        case 4: SearchNum(); break;
        case 5: SearchName(); break;
        case 6: CompareNumMain(); break;
        case 7: CompareScoreMain(); break;
        case 0: Quit(); break;
        default: break;
    }
}*/
int main()
{
    while (true)
    {
        Input();
    }
}