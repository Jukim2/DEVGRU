#include "CCTVManager.h"
#include "CCTVWidget.h"
#include "Blueprint/UserWidget.h"
#include "Project/Character/ShooterCharacter.h"
#include "Project/Controller/ShooterController.h"
#include "Project/ShooterTypes/ItemTypes.h"

#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/CinematicCamera/Public/CineCameraActor.h"

ACCTVManager::ACCTVManager()
{
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	RootComponent = SkeletalMeshComponent;
	
	SphereCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollisionComponent"));
	SphereCollisionComponent->SetupAttachment(RootComponent);
}

void ACCTVManager::BeginPlay()
{
	Super::BeginPlay();

	SphereCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereCollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	SphereCollisionComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
	SphereCollisionComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACineCameraActor::StaticClass(), CameraActors);

	InputComponent = NewObject<UInputComponent>(this);
	InputComponent->RegisterComponent();
	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::Right, IE_Pressed, this, &ACCTVManager::WatchNextCCTV);
		InputComponent->BindKey(EKeys::Left, IE_Pressed, this, &ACCTVManager::WatchPrevCCTV);
		EnableInput(GetWorld()->GetFirstPlayerController());
	}
}

void ACCTVManager::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (!ShooterCharacter) return;
	
	if (ShooterCharacter->IsLocallyControlled())
	{
		ShooterCharacter->AddOverlappingInteractable(this);
	}
}

void ACCTVManager::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (!ShooterCharacter) return;

	if (ShooterCharacter->IsLocallyControlled())
	{
		ShooterCharacter->RemoveOverlappingInteractable(this);
	}
}

void ACCTVManager::Interact(AActor* Interactor)
{
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(Interactor);
	if (ShooterCharacter && ShooterCharacter->CheckInventory(EItemType::EIT_Key))
	{
		if (bInteracting)
		{
			TurnOffCCTV(ShooterCharacter);
		}
		else
		{
			TurnOnCCTV(ShooterCharacter);
		}
	}
}


void ACCTVManager::TurnOffCCTV(AShooterCharacter* ShooterCharacter)
{
	ShooterController = Cast<AShooterController>(ShooterCharacter->Controller);
	if (!ShooterController) return;
	
	ShooterController->SetViewTargetWithBlend(ShooterCharacter, 0.1f);
	ShooterCharacter->SetbDisableGameplay(false);
	ShooterController->ShowHUD();
	CCTVWidget->SetVisibility(ESlateVisibility::Collapsed);
	bInteracting = false;
}

void ACCTVManager::TurnOnCCTV(AShooterCharacter* ShooterCharacter)
{
	ShooterController = Cast<AShooterController>(ShooterCharacter->Controller);
	if (CameraActors.Num() == 0 || !ShooterController) return;
	
	CurrentCameraIndex = 0;
	ShooterController->SetViewTargetWithBlend(CameraActors[CurrentCameraIndex], 0.1f);
	ShooterCharacter->SetbDisableGameplay(true);
	ShooterController->HideHUD();
	AddCCTVWidget();
	CCTVWidget->SetCCTVNum(CurrentCameraIndex);
	bInteracting = true;
}

void ACCTVManager::WatchNextCCTV()
{
	if (bInteracting)
	{
		CurrentCameraIndex = (CurrentCameraIndex + 1) % CameraActors.Num();
		CCTVWidget->SetCCTVNum(CurrentCameraIndex);
		ShooterController->SetViewTarget(CameraActors[CurrentCameraIndex]);
	}
}

void ACCTVManager::WatchPrevCCTV()
{
	if (bInteracting)
	{
		CurrentCameraIndex = (CurrentCameraIndex - 1 + CameraActors.Num()) % CameraActors.Num();
		CCTVWidget->SetCCTVNum(CurrentCameraIndex);
		ShooterController->SetViewTarget(CameraActors[CurrentCameraIndex]);
	}
}

void ACCTVManager::AddCCTVWidget()
{
	if (!CCTVWidget)
	{
		CCTVWidget = CreateWidget<UCCTVWidget>(ShooterController, CCTVWidgetClass);
		CCTVWidget->AddToViewport();
	}
	else
	{
		CCTVWidget->SetVisibility(ESlateVisibility::Visible);
	}
}
