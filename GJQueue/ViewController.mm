//
//  ViewController.m
//  GJQueue
//
//  Created by tongguan on 16/3/15.
//  Copyright © 2016年 MinorUncle. All rights reserved.
//

#import "ViewController.h"
#import "GJQueue.h"
typedef struct st{
    int a;
    char* c;
    
}st;

@interface ViewController ()
{
    bool run;
    GJQueue<st> _queue;
    st _s;
    dispatch_queue_t popq;
    dispatch_queue_t pushq;
}
@end

@implementation ViewController
static void popC(st* d,st* s){
    d->c = (char*)malloc(strlen(s->c)+1);
    memcpy(d->c, s->c, strlen(s->c)+1);
    d->a = s->a;
    free(s->c);
}
static void pushC(st* d,st* s){
    d->c = (char*)malloc(strlen(s->c)+1);
    memcpy(d->c, s->c, strlen(s->c)+1);
    d->a = s->a;
}
- (void)viewDidLoad {
    [super viewDidLoad];
    
    run = YES;
    _queue.pushCopyBlock = pushC;
    _queue.popCopyBlock = popC;
    _queue.shouldWait = YES;
    _queue.shouldNonatomic = true;
    
    popq = dispatch_queue_create("pop", DISPATCH_QUEUE_CONCURRENT);
    pushq = dispatch_queue_create("push", DISPATCH_QUEUE_CONCURRENT);
    UIButton* btn = [[UIButton alloc]initWithFrame:(CGRect){100,100,100,60}];
    [btn setBackgroundColor:[UIColor greenColor]];
    [btn setTitle:@"push" forState:UIControlStateNormal];
    [self.view addSubview:btn];
    [btn addTarget:self action:@selector(push) forControlEvents:UIControlEventTouchUpInside];
    
    btn = [[UIButton alloc]initWithFrame:(CGRect){100,200,100,60}];
    [self.view addSubview:btn];
    [btn setBackgroundColor:[UIColor greenColor]];
    [btn addTarget:self action:@selector(pop) forControlEvents:UIControlEventTouchUpInside];
    [btn setTitle:@"pop" forState:UIControlStateNormal];    // Do any additional setup after loading the view, typically from a nib.
    dispatch_async(pushq, ^{
        while (run) {
            [self push];
            sleep(0.05);
        }
        
    });
    dispatch_async(popq, ^{
        while (run) {
            [self pop];
            sleep(0.05);
        }
    });
}

-(void)push
{
   
        __block st t;
        static int i;
        char a[10] = " sefd";
        t.c = a;
        t.a = i++;
    _queue.queueCopyPush(&t);
//    NSLog(@"push %d",i);
}
-(void)pop{
//    int *j ;
//    _queue->queueRetainPop(&j);
    
    
        st t;
        if (_queue.queueCopyPop(&t)) {
            NSLog(@"pop:%d",t.a);
        };

}

-(void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event{
    run = !run;
}
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
