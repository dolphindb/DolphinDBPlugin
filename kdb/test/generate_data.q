BASEDIR:hsym`$system"cd";
DATADIR:.Q.dd[BASEDIR]`data;

// 测试各种数据类型是否支持
N:10;
Types:([]
  bool     : N?0b ;
  uuid     : N?0Ng;
  byte     : N?0x0;
  short    :(N?0Wh)*N?-1 1h;
  int      :(N?0Wi)*N?-1 1i;
  long     :(N?0W )*N?-1 1 ;
  float    :(N?"e"$0Wh)*N?-1 1;
  double   :(N?"f"$0Wh)*N?-1 1;
  char     : N?.Q.b6;
  str      :@[` sv/:k?'`$string k:1+N?8;1?N;:;`];
  nanostamp:(N?    .z.P)*N?-1 1;
  month    :(N?"m"$.z.D)*N?-1 1;
  date     :(N?    .z.D)*N?-1 1;
  timestamp:(N?    .z.Z)*N?-1 1;
  nano     : N?    .z.N;
  minute   : N?"u"$.z.T;
  second   : N?"v"$.z.T;
  time     : N?    .z.T );

Types:.Q.en[DATADIR] @[;`uuid;`u#]
  update charXs:string each str from .[;(2;::);first 0#]
    .[;(3;`byte`short`int`long`float`double`char`nanostamp`month`date`timestamp`nano`minute`second`time);{abs[type x]$0w*abs x}]
      .[;(4;`short`int`long`float`double`nanostamp`month`date`timestamp);{neg abs[type x]$0w*abs x}]
        Types;
show meta Types;

0N!.Q.dd[DATADIR;`Types`       ] set Types;
0N!.Q.dd[DATADIR;`Types0`      ] set 0#Types;
0N!.Q.dd[DATADIR;`Types_no_sym`] set delete str from Types;
0N!.Q.dd[DATADIR;`Types_any`   ] set
  delete c from`c xasc update c:(count')charXs, charX0:0#/:charXs from Types;

// 不同的压缩级别
splay:{[name;tab;lbs;alg;lvl]
  :0N!(.Q.dd[DATADIR]`$raze("_"sv string(name;lbs;alg;lvl);"/");lbs;alg;lvl) set tab;
 };
splay[`Types ;  Types;17;2;] each til 10;
splay[`Types0;0#Types;17;2;] each til 10;

// 不同的压缩块大小
splay[`Types ;  Types;;2;6] each (16+til 5)except 17;
splay[`Types0;0#Types;;2;6] each (16+til 5)except 17;

//////////////////////////////////////////////////////////////////////////////

// 测试复杂数据类型是否支持
Arrays:([]
  bools     :(neg[N]?N)?\:0b ;
  uuids     :(neg[N]?N)?\:0Ng;
  bytes     :(neg[N]?N)?\:0x0;
  shorts    :(k?\:0Wh)*(k:neg[N]?N)?\:-1 1h;
  ints      :(k?\:0Wi)*(k:neg[N]?N)?\:-1 1i;
  longs     :(k?\:0W )*(k:neg[N]?N)?\:-1 1 ;
  floats    :(k?\:"e"$0Wh)*(k:neg[N]?N)?\:-1 1;
  doubles   :(k?\:"f"$0Wh)*(k:neg[N]?N)?\:-1 1;
  chars     :(N*neg[N]?N)?\:.Q.b6;
  strs      :(N*neg[N]?N)?\:Types`str;
  nanostamps:(k?\:    .z.P)*(k:neg[N]?N)?\:-1 1;
  months    :(k?\:"m"$.z.D)*(k:neg[N]?N)?\:-1 1;
  dates     :(k?\:    .z.D)*(k:neg[N]?N)?\:-1 1;
  timestamps:(k?\:    .z.Z)*(k:neg[N]?N)?\:-1 1;
  nanos     :(k:neg[N]?N)?\:    .z.N;
  minutes   :(k:neg[N]?N)?\:"u"$.z.T;
  seconds   :(k:neg[N]?N)?\:"v"$.z.T;
  times     :(k:neg[N]?N)?\:    .z.T );

Arrays:.Q.en[DATADIR]
  update charsXs:string strs from Arrays;
show meta Arrays;

0N!.Q.dd[DATADIR;`Arrays`       ] set Arrays;
0N!.Q.dd[DATADIR;`Arrays_no_sym`] set delete strs from Arrays;
0N!.Q.dd[DATADIR;`Arrays_any`   ] set
  delete c from`c xasc update c:(count')charsXs, charsX0:0#/:charsXs from Arrays;

// 不同的压缩级别
splay[`Arrays;Arrays;17;2;] each til 10;

// 不同的压缩块大小
splay[`Arrays;Arrays;;2;6] each (16+til 5)except 17;

// 复制enum sym，去除几个syms，并压缩
(.Q.dd[DATADIR;`alt_sym];17;2;6) set -5_ get .Q.dd[DATADIR;`sym]

//////////////////////////////////////////////////////////////////////////////
// 加载所有样本表
system"l ",1_string DATADIR
\v

// 动态加载
-21!.Q.dd[DATADIR;`Types_17_2_6`.d]

select from .Q.dd[DATADIR;`Types_no_sym`]

sym:get .Q.dd[DATADIR;`sym];
select from .Q.dd[DATADIR;`Types`]

select from .Q.dd[DATADIR;`Types0`]

select from .Q.dd[DATADIR;`Types_any]

count'[k]!k:first first
select charsXs, c:count each charsXs from
select from .Q.dd[DATADIR;`Arrays_no_sym`]

sym:get .Q.dd[DATADIR;`sym];
select from .Q.dd[DATADIR;`Arrays`]

select from .Q.dd[DATADIR;`Arrays_any`]