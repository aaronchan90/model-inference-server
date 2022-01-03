## 通用模型推理服务框架


**开发计划**
  - [x] main.cc && service_config, 实现基本启/停流程以及全局配置解析；（2021-12-31）
  - [x] 全局结构划分；（2022-01-03）
  - [ ] inference_core 以及 inference_payload 定义；（2022-01-03）
    - [x] status.h 定义及实现；
    - [x] inference_core 定义；
    - [ ] inference_payload 定义及实现；
      - [ ] memory_collector
    - [x] proto 定义及实现；
  - [ ] grpc_server 定义及实现；（2022-01-04）
    - [ ] infer_server
    - [ ] status_server
    - [ ] health_server
  - [ ] model_manager 定义及实现；（2022-01-04）
    - [ ] model_manager
    - [ ] scheduler
  - [ ] backend_factory实现；（2022-01-05）