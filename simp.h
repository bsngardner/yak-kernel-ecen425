//H file for simptris game

struct piece {
  int id;
  int type;
  int x;
  int rot;
  int trans;
  struct piece* next;
};


struct bin{
  int flat;
  int ht;
  int x;
};

extern YKQ* new_q;
extern YKSEM* cmd_sem;
extern struct bin bin[];
