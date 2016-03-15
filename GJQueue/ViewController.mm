//
//  ViewController.m
//  GJQueue
//
//  Created by tongguan on 16/3/15.
//  Copyright © 2016年 MinorUncle. All rights reserved.
//

#import "ViewController.h"
#import "GJQueue.h"


@interface ViewController ()
{
    GJQueue<int>* _queue;
}
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    _queue = new GJQueue<int>();
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
}

-(void)push
{   static int i;
    i++;
    _queue->queuePush(i);
    NSLog(@"push");
}
-(void)pop{
//    int *j ;
//    _queue->queueRetainPop(&j);
    
    NSLog(@"pop:%d",_queue->queuePop(NULL));
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
