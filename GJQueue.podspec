#CXLSlideList.podspec
Pod::Spec.new do |s|
  s.name         = "GJQueue"
  s.version      = "1.0.0"
  s.summary      = "a support mutable thread,support deep copy cache and assign cache."

  s.homepage     = "https://github.com/MinorUncle/queue"
  s.author       = { "MinorUncle" => "@1065517719@gmail.com" }
  s.platform     = :ios, "7.0"
  s.ios.deployment_target = "7.0"
  s.source       = { :git => "https://github.com/MinorUncle/queue.git", :tag => s.version}
  s.source_files  = '*'
  s.requires_arc = true
end
