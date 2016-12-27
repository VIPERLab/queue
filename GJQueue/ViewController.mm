//
//  ViewController.m
//  GJQueue
//
//  Created by tongguan on 16/3/15.
//  Copyright © 2016年 MinorUncle. All rights reserved.
//

#import "ViewController.h"
#import "GJQueue+cplus.h"
#import "GJBufferPool+cplus.h"
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
    NSDate* date;
}
@end
@implementation ViewController

-(void)queueTest{
    date = [NSDate date];
    run = YES;
    _queue.autoResize = false;

    
    //push pop不同线程 线程睡0.2
    //autoresize = false,wait = yes nonatomic = yes    9239.936629/s
    //autoresize = true,wait = yes nonatomic = yes  8105.421133/s
    //autoresize = true,wait = no nonatomic = yes 14365.428890
    //autoresize = false,wait = no nonatomic = yes 37086.887812
    
    //push pop相同线程 线程睡0.2
    //autoresize = false,wait = no nonatomic = yes                  11015.503520
    //autoresize = false,wait = no nonatomic = no                  10523.068085
    //autoresize = false,wait = yes nonatomic = yes                  10078.879488
    //autoresize = false,wait = yes nonatomic = no                  9621.923693
    
    
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
            usleep(200);
        }
    });
        dispatch_async(popq, ^{
            while (run) {
                [self pop];
                usleep(250);
            }
        });
}

-(void)poolTest{

    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        GJBufferPool pool;
        GJQueue<GJBuffer*>queue;
        run=YES;
        while (run) {
            int random = arc4random()%4;
            NSLog(@"random:%d,",random);
            if (random!=0) {
                GJBuffer* buffer = pool.getBuffer(40+random*random);
                NSString* string = [NSString stringWithFormat:@"bufferSize:%d",buffer->size];
                memcpy(buffer->data, string.UTF8String, string.length+1);
                queue.queuePush(buffer);
            }else{
                GJBuffer* buffer;
                if (queue.queuePop(&buffer)) {
                    NSLog(@"POP %s",(char*)buffer->data);
                    pool.setBuffer(buffer);
                }else{
                    NSLog(@"NO POP");
                }
                
            }
            sleep(1);
        }
    });
}

- (void)viewDidLoad {
    [super viewDidLoad];
//    [self queueTest];
    [self poolTest];
}
static int i;
-(void)push
{
        __block st t;
//        char a[10] = " sefd";
//        t.c = a;
//        t.a = i++;
//        NSDate* locdate = [NSDate date];
//        _queue.queuePush(t,1000);
//        NSLog(@"date:%f",[[NSDate date]timeIntervalSinceDate:locdate]);
   


//    NSLog(@"push %d",i);
}
-(void)pop{
////    int *j ;
////    _queue->queueRetainPop(&j);
//    
//    
//        st t;
//        if (_queue.queuePop(&t)) {
//            NSLog(@"pop:%d",t.a);
//        };

}
//7308/s
-(void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event{
    run = !run;
    
    NSLog(@"rate:%f\n\n\n",i/[[NSDate date]timeIntervalSinceDate:date]);
}
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
