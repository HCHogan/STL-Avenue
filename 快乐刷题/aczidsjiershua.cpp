#include<iostream>
#include<queue>
using namespace std;
const int N=10010,INF=0x3f3f3f3f;

int son[N][26],idx,ne[N],n,m,cnt[N];
char str[N];

void insert(char* str)
{
    int p=0;
    for(int i=1;str[i];i++)
    {
        int u=str[i]-'a';
        if(!son[p][u]) son[p][u]=++idx;
        p=son[p][u];
    }
    cnt[p]++;
}

void build()
{
    queue<int> q;
    for(int i=0;i<26;i++)
    {
        if(son[0][i]) q.push(son[0][i]);
        while(!q.empty())
        {
            int j=q.front();
            q.pop();
            for(int i=0;i<26;i++)
            {
                if(son[j][i])
                {
                    ne[son[j][i]]=son[ne[j]][i];
                    q.push(son[j][i]);
                }
                else son[j][i]=son[ne[j]][i];
            }
        }
    }
}

int ask(char* str)
{
    int p=0,res=0;
    for(int i=1;str[i];i++)
    {
        int u=str[i]-'a';
        p=son[p][u];
        for(int k=p;k&&~cnt[k];k=ne[k])
        {
            res+=cnt[k];
            cnt[k]=-1;
        }
    }
    return res;
}

int main()
{
    cin>>n>>m;
    for(int i=1;i<=n;i++)
    {
        cin>>str+1;
        insert(str);
    }
    int maxnum=-INF;
    for(int i=1;i<=m;i++)
    {
        cin>>str+1;
        maxnum=max(maxnum,ask(str));
    }
    cout<<maxnum<<endl;
    return 0;
}