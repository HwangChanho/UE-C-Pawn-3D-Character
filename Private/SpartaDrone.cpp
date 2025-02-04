// Fill out your copyright notice in the Description page of Project Settings.

#include "SpartaDrone.h"
#include "SpartaDroneController.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "EnhancedInputComponent.h"

ASpartaDrone::ASpartaDrone()
{
	PrimaryActorTick.bCanEverTick = true;

	// ĸ�� �ݸ��� ����
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->SetupAttachment(RootComponent);
	CapsuleComp->InitCapsuleSize(55.0f, 96.0f);

	// �ݸ��� Ȱ��ȭ
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Block);

	RootComponent = CapsuleComp;

	// �޽� ����
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SkeletalMeshComp->SetupAttachment(CapsuleComp);
	// ���� Ȱ��ȭ
	//SkeletalMeshComp->SetSimulatePhysics(true);

	// ������ �� ����
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = true;

	// ī�޶� ����
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	MaxDroneEnginePower = 1200.0f;
	DroneEnginePower = 0.0f;
	GravityAccel = 980.0f;
	TargetRotation = GetActorRotation();
	AccumulatedRotation = FRotator::ZeroRotator;

	// ����� ���� ȸ���ϵ��� ����
	//bUseControllerRotationPitch = false;
	//bUseControllerRotationYaw = false;
	//bUseControllerRotationRoll = false;

	// SpringArm�� ������ ȸ���� �������� ����
	if (SpringArmComp)
	{
		SpringArmComp->bUsePawnControlRotation = false;
	}
}

void ASpartaDrone::BeginPlay()
{
	Super::BeginPlay();
}

// Physics Pipeline
void ASpartaDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// �ڿ������� ȸ���� ���� ���� ���� (�ٷ� ���� �ȵǰ� �����Ӹ��� ������ġ ����)
	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 5.0f);
	SetActorRotation(NewRotation);

	ApplyTiltEffect(DeltaTime);
	UpdateCamera(DeltaTime);
	SetGravity(DeltaTime);
	ReduceEnginePower(DeltaTime);
	IsGrounded();
}

void ASpartaDrone::ApplyTiltEffect(float DeltaTime)
{
	if (bIsGrounded)
	{
		RestoreTilt(DeltaTime);
		return;
	}

	float MaxTiltAngle = 20.0f;
	float MaxPitchAngle = 10.0f;

	float TargetRoll = CurrentMoveAxisValue * MaxTiltAngle;
	float TargetPitch = CurrentMoveForwardAxis * MaxPitchAngle;

	FRotator CurrentRotation = GetActorRotation();
	FRotator Rotation = FRotator(CurrentRotation.Pitch + TargetPitch, CurrentRotation.Yaw, TargetRoll);

	// ȸ��(Rotation) �� �ε巴�� ����
	/*
	FRotator FMath::RInterpTo(
    FRotator Current,     // ���� ȸ�� ��
    FRotator Target,      // ��ǥ ȸ�� ��
    float DeltaTime,      // ������ �� �ð� (Tick���� ���޵Ǵ� DeltaTime)
    float InterpSpeed     // ���� �ӵ� (���� Ŭ���� ������ ��ǥ�� ����)
	);
	*/
	FRotator SmoothedRotation = FMath::RInterpTo(CurrentRotation, Rotation, DeltaTime, 5.0f);
	SetActorRotation(SmoothedRotation);
}

bool ASpartaDrone::IsGrounded()
{
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0, 0, 10.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);

	if (bHit)
	{
		bIsGrounded = true;
	}
	else
	{
		bIsGrounded = false;
	}

	return bIsGrounded;
}

void ASpartaDrone::RestoreTilt(float DeltaTime)
{
	FRotator RestoreRotation = FRotator::ZeroRotator;

	AccumulatedRotation = FMath::RInterpTo(AccumulatedRotation, RestoreRotation, DeltaTime, 3.0f);

	FRotator CurrentRotation = GetActorRotation();
	FRotator SmoothedRotation = FRotator(AccumulatedRotation.Pitch, CurrentRotation.Yaw, AccumulatedRotation.Roll);

	SetActorRotation(SmoothedRotation);
}

void ASpartaDrone::UpdateCamera(float DeltaTime)
{
	if (!SpringArmComp || !CameraComp) return;

	FRotator TargetCameraRotation = GetActorRotation();
	FRotator SmoothedRotation = FMath::RInterpTo(SpringArmComp->GetComponentRotation(), TargetCameraRotation, DeltaTime, 5.0f);

	//UE_LOG(LogTemp, Warning, TEXT("UpdateCamera[%s]"), *SmoothedRotation.ToString());

	SpringArmComp->SetWorldRotation(SmoothedRotation);
}

void ASpartaDrone::ReduceEnginePower(float DeltaTime)
{
	// ���� �Ŀ� ������ ����
	if (DroneEnginePower > 0.0f)
	{
		DroneEnginePower -= ReducingPower * DeltaTime;
		DroneEnginePower = FMath::Max(DroneEnginePower, 0.0f);
	}
}

void ASpartaDrone::SetGravity(float DeltaTime)
{
	float GravityValue = -GravityAccel * DeltaTime;

	// ���� �Ŀ��� ���� �߷� ��� ����
	float GravityReductionRatio = FMath::Clamp(DroneEnginePower / MaxDroneEnginePower, 0.0f, 1.0f);
	float AdjustedGravity = GravityValue * (1.0f - GravityReductionRatio);
	// UE_LOG(LogTemp, Warning, TEXT("SetGravity = [%f]"), GravityValue); // -16.xxx

	FVector GravityOffset(0, 0, AdjustedGravity);
	FVector NewLocation = GetActorLocation() + GravityOffset;

	if (NewLocation.Z < 0.0f)
	{
		NewLocation.Z = 0.0f;
	}

	SetActorLocation(NewLocation, true);

	//UE_LOG(LogTemp, Log, TEXT("GravityValue: %f, Reduction: %f, AdjustedGravity: %f, DroneEnginePower: %f"), GravityValue, GravityReductionRatio, AdjustedGravity, DroneEnginePower);
}

// ��/�� �̵� (space, shift)
void ASpartaDrone::MoveUp(const FInputActionValue& value)
{
	if (!Controller) return;

	const float AxisValue = value.Get<float>(); // +1, -1 �� ���⼺�� ����

	//UE_LOG(LogTemp, Log, TEXT("%s"), (AxisValue == 1.0f) ? TEXT("Move Up") : (AxisValue == -1.0f) ? TEXT("Move Down") : TEXT("Idle"));

	float DeltaTime = GetWorld()->DeltaTimeSeconds;

	if (DroneEnginePower < MaxDroneEnginePower)
	{
		DroneEnginePower += 10.0f;
	}

	//UE_LOG(LogTemp, Log, TEXT("Engine Power [%f]"), DroneEnginePower);

	FVector UpOffset(0, 0, AxisValue * DroneEnginePower * DeltaTime);
	FVector NewLocation = GetActorLocation() + UpOffset;

	if (FMath::IsNearlyZero(NewLocation.Z, 0.01f))
	{
		NewLocation.Z = 0.0f; // ���� ���� ����
	}

	SetActorLocation(NewLocation, true);
}

void ASpartaDrone::MoveForward(const FInputActionValue& value)
{
	if (!Controller) return;

	float DeltaTime = GetWorld()->DeltaTimeSeconds;

	const float AxisValue = value.Get<float>();
	CurrentMoveForwardAxis = AxisValue;

	UE_LOG(LogTemp, Warning, TEXT("MoveForward[%f]"), AxisValue);

	FRotator CurrentRotation = TargetRotation;
	FVector ForwardDirection = CurrentRotation.Vector();
	
	FVector MoveOffset = ForwardDirection * AxisValue * DroneEnginePower * DeltaTime;

	float GravityFactor = FVector::DotProduct(ForwardDirection, FVector::UpVector); // ���� ����
	MoveOffset.Z -= GravityFactor * 980.0f * DeltaTime;

	FVector NewLocation = GetActorLocation() + MoveOffset;

	UE_LOG(LogTemp, Warning, TEXT("CurrentRotation[%s]"), *CurrentRotation.ToString());
	UE_LOG(LogTemp, Warning, TEXT("MoveOffset Z [%f]"), MoveOffset.Z);

	SetActorLocation(NewLocation, true);
}

void ASpartaDrone::MoveRight(const FInputActionValue& value)
{
	if (!Controller) return;

	float DeltaTime = GetWorld()->DeltaTimeSeconds;
	const float AxisValue = value.Get<float>();
	CurrentMoveAxisValue = AxisValue;

	UE_LOG(LogTemp, Warning, TEXT("MoveRight"));

	FRotator CurrentRotation = TargetRotation;
	FVector RightDirection = FRotationMatrix(CurrentRotation).GetUnitAxis(EAxis::Y);

	FVector MoveOffset = RightDirection * AxisValue * DroneEnginePower * DeltaTime;
	FVector NewLocation = GetActorLocation() + MoveOffset;

	SetActorLocation(NewLocation, true);
}

void ASpartaDrone::LookPitch(const FInputActionValue& value)
{
	if (!Controller) return;

	UE_LOG(LogTemp, Warning, TEXT("LookPitch"));

	float AxisValue = value.Get<float>();
	if (FMath::IsNearlyZero(AxisValue)) return;

	float CurrentPitch = TargetRotation.Pitch;

	float NewPitch = CurrentPitch + (AxisValue * PitchSpeed * GetWorld()->GetDeltaSeconds());

	TargetRotation.Pitch = FMath::Clamp(NewPitch, MinPitch, MaxPitch);
}

void ASpartaDrone::LookRoll(const FInputActionValue& value)
{
	if (!Controller) return;

	UE_LOG(LogTemp, Warning, TEXT("LookRoll"));
}

void ASpartaDrone::LookYaw(const FInputActionValue& value)
{
	if (!Controller) return;

	UE_LOG(LogTemp, Warning, TEXT("LookYaw"));

	float AxisValue = value.Get<float>();
	if (FMath::IsNearlyZero(AxisValue)) return;

	float CurrentYaw = TargetRotation.Yaw;

	float NewYaw = CurrentYaw + (AxisValue * YawSpeed * GetWorld()->GetDeltaSeconds());

	// ���ѵ� ������ Ŭ����
	TargetRotation.Yaw = FMath::Clamp(NewYaw, MinYaw, MaxYaw);
	
}

void ASpartaDrone::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ASpartaDroneController* DroneController = Cast<ASpartaDroneController>(GetController()))
		{
			if (DroneController->MoveUpAction)
			{
				EnhancedInput->BindAction(
					DroneController->MoveUpAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaDrone::MoveUp
				);
			}

			if (DroneController->MoveForwardAction)
			{
				EnhancedInput->BindAction(
					DroneController->MoveForwardAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaDrone::MoveForward
				);
			}

			if (DroneController->MoveRightAction)
			{
				EnhancedInput->BindAction(
					DroneController->MoveRightAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaDrone::MoveRight
				);
			}

			if (DroneController->LookPitchAction)
			{
				EnhancedInput->BindAction(
					DroneController->LookPitchAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaDrone::LookPitch
				);
			}

			if (DroneController->LookRollAction)
			{
				EnhancedInput->BindAction(
					DroneController->LookRollAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaDrone::LookRoll
				);
			}

			if (DroneController->LookYawAction)
			{
				EnhancedInput->BindAction(
					DroneController->LookYawAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaDrone::LookYaw
				);
			}
		}
	}
}