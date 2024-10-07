## Preview

| ![image](https://github.com/user-attachments/assets/0f484291-a0b1-4ad0-bab8-a637d4ae5628) | ![image](https://github.com/user-attachments/assets/608e3da5-3ee1-4ff2-a0bd-3174056784e7) | ![image](https://github.com/user-attachments/assets/cd68a52c-6885-4c8c-b26b-8668be3c680c) |
| --- | --- | --- |
| ![image](https://github.com/user-attachments/assets/337975e8-0ac4-4d71-a878-2f255f740b72) | ![image](https://github.com/user-attachments/assets/5faf27d1-47b7-4b35-b004-f58869859a47) | ![image](https://github.com/user-attachments/assets/8e8d0ac3-55cc-46c5-a692-170995b61a11) |
| ![image](https://github.com/user-attachments/assets/912582d3-2511-4e2d-8087-99b14f32956f) | ![image](https://github.com/user-attachments/assets/0dc0e95b-2cf2-469e-b4c1-459d71d1497c) | ![image](https://github.com/user-attachments/assets/f28fc06c-311d-491c-a6a4-909b40c665e4) | 


# DEVGRU 게임

- **개발 기간 : 2024.07.14 - 2024.09.23**
- **개발 인원 : 2명**
    - **(프로그래머)**: 게임 시스템 전반에 걸쳐 구현 담당.
    - **(프로그래머)**: 무기 시스템 및 관련 애니메이션 구현을 중점적으로 담당.
- **엔진 버전 : Unreal 5.4.2**
- **프로젝트 소개**
    - 멀티플레이어 fps장르로 군사 기지에서 상대방을 섬멸하는 것을 목표로 하는 게임.
- **C++ / 블루프린트 비중**
    - 타임라인, 카메라 흔들림 등에 블루프린트 최소한으로 사용
    - 그 외 기능적인 부분들은 C++로 구현
- **목차**
    1. **주요 기능 소개**
    2. **전체 게임 진행 소개**
    3. **세부 개발 기능 소개**
- Youtube Link : [https://youtu.be/Ss7w1ldoopc](https://youtu.be/Ss7w1ldoopc)


## **1. 주요기능 소개**

### **a. 캐릭터 구현**

- **구조 :** Character 클래스와 하위 Component
    <div align="center">
    <img src="https://github.com/user-attachments/assets/0ab5eb63-0d5e-4c59-bf70-ba59689a6a34" alt="Character"       style="width:70%;">
    </div>
    
- **하위 Component 예시**
    1. Health
    2. Movement
    3. WeaponManager
    4. WeaponAction
    5. CharacterAnimation
    6. Inventory
- **구현**
    - 무기 관련 동작, 애니메이션, 움직임 등 각 기능을 독립된 컴포넌트로 분리하여, 각각의 컴포넌트가 자신의 역할에 맞는 동작을 처리할 수 있도록 구현.
    - 재장전이나 무기 교체처럼 애니메이션 몽타주가 함께 실행되어야 하는 경우, **WeaponActionCompo
    nent**에서 해당 동작을 처리하며 Character 클래스의 **CharacterState**를 업데이트. Chcaracter
    State에 따라 다른 컴포넌트(예: 애니메이션 컴포넌트)에서 후속 동작을 수행.
    - 예시 흐름: **WeaponActionComponent**에서 **Reload()** 실행 → **CharacterState**가 **ECS_Reloading**으로 변경 → **CharacterAnimationComponent**의 **PlayReloadMontage()**로 ****재장전 애니메이션 몽타주를 재생.
- **취지**
    - 각 **Component**의 책임을 명확히 분리하여, 특정 기능을 독립적으로 처리할 수 있도록 모듈화.
    - 서로 관련된 **Component**는 **CharacterState** 변수를 통해 연계하여, 컴포넌트 간 의존성을 최소화.
    - 네트워크 부하를 최적화하기 위해 **Multicast** 함수 호출 대신 **CharacterState** 변수만 **Replicate**하여 동기화.
    

### b. 총기 구현

- **구조**:
    <div align="center">
    <img src="https://github.com/user-attachments/assets/828742a6-28be-495b-8ea8-eb80ead87a30" alt="Character" style="width:50%;">
    </div>
    
    - 모든 무기가 공통으로 가지는 특성을 포함하는 **Base Weapon** 클래스를 생성.
    - **Weapon**의 자식 클래스를 분리:
        - **Ranged Weapon**은 탄약 관리 등 추가적인 특성이 있기 때문에 분리.
    - **Ranged Weapon** 자식 클래스를 추가로 분리:
        - **Gun**은 반동, 탄피 배출, 조준 등의 특성을 가지기 때문에 **Gun**과 **Grenade**로 분리.
    - **Gun** 자식 클래스를 다시 분리:
        - **Rifle**과 **Pistol**은 **Attack()** 메서드가 서로 다르게 구현되므로 분리.
    


- **취지**:
    - 캐릭터는 1~3번 슬롯에 무기를 장착할 수 있으며, 이를 **AWeapon**을 key로 사용하는 **Map** 자료구조로 관리.
    - 따라서 모든 무기는 **Weapon** 클래스여야 하지만, 각 무기의 종류에 따라 고유한 특성이나 공격 방식이 존재하므로, **Base Weapon** 클래스를 기반으로 다양한 클래스를 생성하여 관리.

### c. 인벤토리 구현

- 구조
    <div align="center">
    <img src="https://github.com/user-attachments/assets/d703b1e7-284f-4866-9e91-7ec5911900d8" alt="Character" style="width:50%;">
    </div>
    
    - **Item / Item Object**:
        - **Item**: 게임 내에서 실제로 레벨에 생성된 객체로, **Actor**로 구현됨.
        - **Item Object**: 인벤토리에 아이템을 저장할 때, 실제 **Item** 대신 저장되는 객체로, 아이템에 대한 정보를 담고 있음.
    - **Inventory Component**:
        - 인벤토리 기능을 담당하는 **Item Object** 배열을 관리하며, 아이템 획득, 배치, 드랍 등의 기능을 구현.
    - **Inventory Grid Widget**:
        - 사용자가 시각적으로 볼 수 있는 인벤토리를 구현하는 클래스. **Inventory Component**에 저장된 **Item Object**를 기반으로 각 아이템에 맞는 **Widget**을 생성해 인벤토리를 시각화.


- **기능**:
    - **Spatial Inventory** 시스템: 인벤토리가 여러 격자로 나뉘어 있으며, 각 아이템은 차지하는 공간이 다름. 따라서 아이템의 크기를 고려하여 인벤토리에 저장.
    - 아이템 저장, 위치 이동, 스폰 기능 제공.

## 2. 전체 게임 진행

  <div align="center">
    <img src="https://github.com/user-attachments/assets/92e17a74-d664-4caa-82df-0e7636ff7726" alt="Character" style="width:60%;">
  </div>

- **MainMenu**:
    - **Host / Join**: OnlineSubsystem을 이용하여 세션 생성 및 참여
- **Lobby**:
    - **LobbyGameMode**:
        - PostLogin, Logout을 사용하여 플레이어 수를 카운트.
        - 플레이어가 8명이 모이면 자동으로 게임 레벨로 이동.
- **Game**:
    - **TeamGameMode**:
        - PostLogin으로 게임에 입장한 플레이어들에게 팀을 자동으로 배정.
        - 팀별 스폰 위치 지정 및 같은 팀끼리는 공격 불가.
    - **GameState**:
        - **WaitingToStart**: 각 라운드 시작 전 대기 상태.
        - **InProgress**: 게임 진행 중인 상태.
        - **Cooldown**: 라운드가 끝났는지 확인하고, 끝나지 않았을 경우 변수 초기화 후 유저 리스폰을 통해 라운드 재개.
        - **GameEnd**: 특정 팀이 승리하면 게임이 종료되고 메인 메뉴로 이동.

## **3. 세부 개발 기능**

- **무기**:
    - 다양한 종류의 무기 구현.
    - 무기 교체 및 장착 기능.
    - 총기가 화면 중앙을 향하도록 조정.
    - 총기 반동 시스템 구현.
    - Hand Sway 효과 적용.
    - 재장전 기능.
    - 조준 모드 구현.

- **플레이어 애니메이션**:
    - 무기 종류에 따른 애니메이션 적용.
    - 상태별 애니메이션 (죽음, 앉기 등) 적용.
    - TurnInPlace 기능 구현.
    - 재장전, 무기 교체 등 전투 관련 애니메이션 적용.

- **HUD**:
    - 플레이 시간 표시.
    - 팀별 스코어 표시.
    - 현재 보유 및 장착 중인 무기의 UI 표시.
    - 무기에 장착된 탄약과 보유 중인 탄약 표시.
    - 플레이어의 체력 표시.
      
- **인벤토리**
    - Spatial Inventory(공간형 인벤토리) 개발.
    - 상호작용 키를 사용해 아이템 획득 시 자동 배치.
    - 드래그 앤 드롭을 이용한 아이템 재배치, 회전, 스폰 기능 제공.

- **멀티플레이 세션**:
    - OnlineSubsystem을 이용한 세션 구현.
    - 메인 메뉴 버튼과 연동하여 세션 시작 및 참가 기능 제공.
      
- **관전자 모드 (Spectator)**:
    - 플레이어가 죽은 후 관전 기능 제공.
      
- **상호작용**
    - F 키를 이용한 상호작용 기능.
    - 상호작용 가능한 클래스는 IInteractable 인터페이스 구현.
    - 각 클래스가 구현한 함수 내용에 따라 CCTV 시청 또는 아이템 획득 등의 다른 동작 수행.
