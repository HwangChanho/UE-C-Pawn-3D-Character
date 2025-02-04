// Fill out your copyright notice in the Description page of Project Settings.

#include "SpartaPawn.h"
#include "SpartaPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

DEFINE_LOG_CATEGORY(LogAAA);

ASpartaPawn::ASpartaPawn()
{
 	PrimaryActorTick.bCanEverTick = true;

    // 캡슐 콜리전 생성
    CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
    CapsuleComp->SetupAttachment(RootComponent);
    CapsuleComp->InitCapsuleSize(55.0f, 96.0f);
	RootComponent = CapsuleComp;

	// 캡슐의 물리 시뮬레이션 비활성화
	CapsuleComp->SetSimulatePhysics(false);
	CapsuleComp->SetEnableGravity(false);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 메쉬 생성
    SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
    SkeletalMeshComp->SetupAttachment(CapsuleComp);

    // 스프링 암 생성
    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
    SpringArmComp->SetupAttachment(RootComponent);
    SpringArmComp->bUsePawnControlRotation = true;

    // 카메라 생성
    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
    CameraComp->SetupAttachment(SpringArmComp);

	WalkingSpeed = 600.0f;
	SprintSpeed = 1200.0f;

	bIsJumping = false;
	Velocity = FVector::ZeroVector;
	Gravity = -980.f;
	bIsMoving = false;

	// Collision
	CapsuleRadius = 50.0f;
	CapsuleHalfHeight = 50.0f;
	BlockedPosition = FVector::ZeroVector;
}

void ASpartaPawn::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogAAA, Warning, TEXT("SpartaPawn"));

	// Collision
	if (UCapsuleComponent* CollisionCapsuleComp = FindComponentByClass<UCapsuleComponent>())
	{
		UE_LOG(LogAAA, Warning, TEXT("UCapsuleComponent Found"));

		CapsuleRadius = CollisionCapsuleComp->GetScaledCapsuleRadius();
		CapsuleHalfHeight = CollisionCapsuleComp->GetScaledCapsuleHalfHeight();
	}
}

void ASpartaPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// damping 

	UpdateFloorZ();   // 바닥 충돌 감지 -> LineTrace
	CheckCollision(); // 벽충돌 감지 -> SweepMultiByObjectType

	// 중력 적용
	Velocity.Z += Gravity * DeltaTime;

	// 이동 계산
	FVector NewLocation = GetActorLocation() + Velocity * DeltaTime;

	// 바닥 충돌 감지
	if (NewLocation.Z <= CurrentFloorZ)
	{
		//UE_LOG(LogAAA, Warning, TEXT("바닥 충돌: %f"), NewLocation.Z);

		NewLocation.Z = CurrentFloorZ;
		bIsJumping = false;
		Velocity.Z = 0.f;
	}

	//UE_LOG(LogAAA, Warning, TEXT("Tick NewLocation: %s"), *NewLocation.ToString());

	SetActorLocation(NewLocation);
}

// 충돌 확인 함수
bool ASpartaPawn::CheckCollision()
{
	FVector StartLocation = GetActorLocation();
	FVector MoveDirection = GetVelocity().GetSafeNormal();

	if (MoveDirection.IsNearlyZero()) {
		MoveDirection = GetActorForwardVector();
	}

	float ZOffset = 80.0f; // 지면충돌떄문에 올렸다 라인 트레이스 꾸역꾸역 쓰려면 어쩔수 없음
	StartLocation.Z += ZOffset;
	FVector EndLocation = StartLocation;

	// 캡슐 콜리전 사용
	float DetectionRadius = CapsuleRadius;
	float DetectionHalfHeight = CapsuleHalfHeight;
	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(DetectionRadius, DetectionHalfHeight);

	TArray<FHitResult> HitResults;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // 자기 자신 무시

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);

	bool bHit = GetWorld()->SweepMultiByObjectType(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ObjectQueryParams,
		CollisionShape,
		QueryParams
	);

	// 디버그: 탐지 범위 시각화
	//DrawDebugCapsule(
	//	GetWorld(),
	//	StartLocation,            // 캡슐 중심
	//	DetectionHalfHeight,      // 반 높이
	//	DetectionRadius,          // 반지름
	//	FQuat::Identity,          // 회전 (없음)
	//	FColor::Green,            // 색상 (녹색)
	//	false,                    // 영구 표시 여부
	//	2.0f,                     // 지속 시간 (2초)
	//	0,                        // 깊이 우선 순위
	//	1.0f                      // 선 두께
	//);

	FVector CurrentVelocity = GetVelocity();

	HitResults.Sort([](const FHitResult& A, const FHitResult& B) {
		return A.Distance < B.Distance;
	});

	if (bHit) {
		for (const FHitResult& Hit : HitResults) {
			if (Hit.bBlockingHit) {
				FVector Normal = Hit.ImpactNormal;
				FVector ActorUp = GetActorUpVector();

				float DotWithUp = FVector::DotProduct(Normal, ActorUp);

				// 지면 판별
				if (DotWithUp > 0.7f) {
					//UE_LOG(LogAAA, Warning, TEXT("지면충돌 무시: %f"), DotWithUp);
					continue;  // 지면 충돌 무시
				}

				// 디버그 시각화
				DrawDebugCapsule(GetWorld(), Hit.ImpactPoint + FVector(0, 0, ZOffset), 100.0f, 50.0f, FQuat::Identity, FColor::Red, false, 2.0f);

				// 반사 벡터 계산
				FVector ReflectedVelocity = CurrentVelocity - 2 * FVector::DotProduct(CurrentVelocity, Normal) * Normal;

				// 속도 감소 적용
				float Elasticity = 0.5f;  // 탄성 계수 감소
				ReflectedVelocity *= Elasticity;

				// 속도 업데이트 (벽 충돌 시 속도 제한)
				CurrentVelocity = ReflectedVelocity;

				// **속도 제한 - 벽에 닿으면 해당 방향 속도 제거**
				FVector ProjectedVelocity = FVector::VectorPlaneProject(CurrentVelocity, Normal);

				//UE_LOG(LogAAA, Warning, TEXT("속도: %s"), *ProjectedVelocity.ToString());

				CurrentVelocity = ProjectedVelocity;

				// 밀어내기
				FVector PushOut = Normal * 10.0f;
				SetActorLocation(GetActorLocation() + PushOut);

				// 디버그 로그
				UE_LOG(LogAAA, Warning, TEXT("충돌한 액터: %s"), *Hit.GetActor()->GetName());
				UE_LOG(LogAAA, Warning, TEXT("법선: %s, 제한 후 속도: %s"), *Normal.ToString(), *CurrentVelocity.ToString());
				UE_LOG(LogAAA, Warning, TEXT("거리: %f"), Hit.Distance);
			}
		}
	}

	return bHit;
}

void ASpartaPawn::UpdateFloorZ()
{
	FVector Start = GetActorLocation();

	// 속도에 따라 동적으로 바닥 감지 거리 설정
	float FallSpeed = FMath::Abs(Velocity.Z);  // 현재 Z축 속도의 절댓값
	float TraceDistance = FMath::Clamp(FallSpeed * 0.1f, 500.f, 10000.f);
	// 속도가 클수록 더 깊이 검사 (최소 100 ~ 최대 1000)

	FVector End = Start - FVector(0.f, 0.f, TraceDistance);
	FHitResult HitResult;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);

	DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 1.f, 0, 2.f);

	if (bHit)
	{
		CurrentFloorZ = HitResult.Location.Z;

		DrawDebugSphere(GetWorld(), HitResult.Location, 5.f, 12, FColor::Blue, false, 1.f);
	}
}

void ASpartaPawn::Move(const FInputActionValue& value)
{
	if (!Controller) return;

	const FVector2D MoveInput = value.Get<FVector2D>();
	if (MoveInput.IsNearlyZero()) return;

	MovementByActorWorldOffset(MoveInput);

	/*
	AddActorLocalOffset(); // 액터의 로컬(Local) 좌표계를 기준으로 위치를 이동시킴
	AddActorWorldOffset(); // 월드(World) 좌표계를 기준으로 액터의 위치를 이동
	AddActorLocalRotation(); // 로컬(Local) 좌표계를 기준으로 액터의 회전을 변경
	*/
}

void ASpartaPawn::MovementByActorWorldOffset(const FVector2D moveInput)
{
	// 컨트롤러의 회전값 가져오기 (Yaw만 사용)
	FRotator ControlRotation = Controller->GetControlRotation();
	FRotator YawRotation(0, ControlRotation.Yaw, 0);

	// 컨트롤러의 Forward(앞), Right(오른쪽) 벡터 계산
	FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// 이동 벡터 계산
	FVector MoveDirection = (ForwardDirection * moveInput.X) + (RightDirection * moveInput.Y);
	
	if (!MoveDirection.IsNearlyZero())
	{
		SetActorRotate(MoveDirection);
	}

	float BaseSpeed = bIsSprinting ? SprintSpeed : WalkingSpeed;
	CurrentSpeed = bIsJumping ? BaseSpeed / 2 : BaseSpeed;


	// 이동 적용 (Tick)
	AddActorWorldOffset(MoveDirection * CurrentSpeed * GetWorld()->GetDeltaSeconds(), true);
}

void ASpartaPawn::SetActorRotate(FVector MoveDirection)
{
	MoveDirection.Normalize(); // 이동 방향 정규화

	// 현재 속도 계산 (이동량 기반)
	float Speed = MoveDirection.Size(); // 이동 벡터의 크기 → 속도 판단 기준
	float SpeedThreshold = 10.0f;

	// 이동 방향을 Rotator로 변환
	FRotator TargetRotation = MoveDirection.Rotation();

	float RotationSpeed = 5.0f;  // 회전 속도 (값이 클수록 빠르게 회전)
	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), RotationSpeed);
	SetActorRotation(NewRotation);
}

void ASpartaPawn::Startjump(const FInputActionValue& value)
{
	if (value.Get<bool>())
	{
		if (!bIsJumping)
		{
			UE_LOG(LogAAA, Warning, TEXT("Startjump"));
			bIsJumping = true;
			Velocity.Z = 600.f;
		}
	}
}

void ASpartaPawn::StopJump(const FInputActionValue& value)
{
	if (!bIsJumping) return;

	if (!value.Get<bool>())
	{
		if (Velocity.Z > 300.f)
		{
			UE_LOG(LogAAA, Warning, TEXT("StopJump Triggered"));
			Velocity.Z = 300.f;
		}
	}
}

void ASpartaPawn::Look(const FInputActionValue& value)
{
	FVector2D LookInput = value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookInput.X);
		AddControllerPitchInput(LookInput.Y);
	}
	else
	{
		UE_LOG(LogAAA, Warning, TEXT("No Controller"));
	}
}

void ASpartaPawn::StartSprint(const FInputActionValue& value)
{
	bIsSprinting = true;
}

void ASpartaPawn::StopSprint(const FInputActionValue& value)
{
	bIsSprinting = false;
}

void ASpartaPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ASpartaPlayerController* PlayerController = Cast<ASpartaPlayerController>(GetController()))
		{
			// Move
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaPawn::Move
				);
			}

			// Sprint
			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaPawn::StartSprint
				);
			}

			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Completed,
					this,
					&ASpartaPawn::StopSprint
				);
			}

			// Look
			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaPawn::Look
				);
			}

			// Jump
			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaPawn::Startjump
				);
			}

			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Completed,
					this,
					&ASpartaPawn::StopJump
				);
			}
		}
	}
}