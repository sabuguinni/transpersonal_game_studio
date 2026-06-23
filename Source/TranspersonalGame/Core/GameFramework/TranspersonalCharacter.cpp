// TranspersonalCharacter.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Prehistoric survival game — full character implementation with SurvivalComponent + BiomeManager wiring

#include "TranspersonalCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ----------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------
ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Survival Component ---
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // --- Camera Boom ---
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // --- Follow Camera ---
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // --- Movement defaults ---
    WalkSpeed = 300.0f;
    RunSpeed  = 600.0f;
    bIsRunning = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    // --- Survival stat mirrors ---
    CurrentHealth   = 100.0f;
    CurrentHunger   = 100.0f;
    CurrentThirst   = 100.0f;
    CurrentStamina  = 100.0f;
    CurrentFear     = 0.0f;

    // --- Biome environment ---
    CurrentBiomeTemperature  = 24.0f;
    CurrentBiomeHazardLevel  = 0.2f;
    BiomeQueryInterval       = 5.0f;
    BiomeQueryTimer          = 0.0f;
}

// ----------------------------------------------------------------
// BeginPlay
// ----------------------------------------------------------------
void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (SurvivalComp)
    {
        SurvivalComp->SetHealth(100.0f);
        SurvivalComp->SetHunger(100.0f);
        SurvivalComp->SetThirst(100.0f);
        SurvivalComp->SetStamina(100.0f);
        SurvivalComp->SetFear(0.0f);
    }
}

// ----------------------------------------------------------------
// Tick — mirror survival stats + biome query + stamina drain on run
// ----------------------------------------------------------------
void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Mirror survival stats from component
    if (SurvivalComp)
    {
        CurrentHealth  = SurvivalComp->GetHealth();
        CurrentHunger  = SurvivalComp->GetHunger();
        CurrentThirst  = SurvivalComp->GetThirst();
        CurrentStamina = SurvivalComp->GetStamina();
        CurrentFear    = SurvivalComp->GetFear();

        // Running drains stamina; walking restores it slowly
        if (bIsRunning && GetVelocity().SizeSquared() > 100.0f)
        {
            float NewStamina = FMath::Max(0.0f, CurrentStamina - 10.0f * DeltaTime);
            SurvivalComp->SetStamina(NewStamina);

            // Force walk if stamina depleted
            if (NewStamina <= 0.0f)
            {
                StopRun();
            }
        }
        else if (!bIsRunning && CurrentStamina < 100.0f)
        {
            float NewStamina = FMath::Min(100.0f, CurrentStamina + 5.0f * DeltaTime);
            SurvivalComp->SetStamina(NewStamina);
        }

        // Biome hazard damages health over time
        if (CurrentBiomeHazardLevel > 0.7f)
        {
            float HazardDamage = (CurrentBiomeHazardLevel - 0.7f) * 5.0f * DeltaTime;
            float NewHealth = FMath::Max(0.0f, CurrentHealth - HazardDamage);
            SurvivalComp->SetHealth(NewHealth);
        }

        // Extreme temperature increases fear
        if (CurrentBiomeTemperature > 50.0f || CurrentBiomeTemperature < 0.0f)
        {
            float TempFear = FMath::Abs(CurrentBiomeTemperature > 50.0f
                ? CurrentBiomeTemperature - 50.0f
                : CurrentBiomeTemperature) * 0.5f * DeltaTime;
            float NewFear = FMath::Min(100.0f, CurrentFear + TempFear);
            SurvivalComp->SetFear(NewFear);
        }
    }

    // Biome query timer
    BiomeQueryTimer += DeltaTime;
    if (BiomeQueryTimer >= BiomeQueryInterval)
    {
        BiomeQueryTimer = 0.0f;
        QueryBiomeEnvironment();
    }
}

// ----------------------------------------------------------------
// QueryBiomeEnvironment — pulls temperature + hazard from BiomeManager
// ----------------------------------------------------------------
void ATranspersonalCharacter::QueryBiomeEnvironment()
{
    // BiomeManager is a UGameInstanceSubsystem — get it via GameInstance
    UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
    if (!GI) return;

    // Use raw pointer (forward-declared) to avoid hard module dependency
    // BiomeManager is discovered at runtime via subsystem lookup
    // This avoids circular includes while keeping the wiring functional
    static const FName BiomeManagerName = TEXT("BiomeManager");

    FVector Location = GetActorLocation();

    // Attempt to find BiomeManager subsystem via reflection
    UClass* BiomeClass = FindObject<UClass>(ANY_PACKAGE, TEXT("BiomeManager"));
    if (BiomeClass)
    {
        UGameInstanceSubsystem* BiomeSys = GI->GetSubsystemBase(BiomeClass);
        if (BiomeSys)
        {
            // Call GetTemperatureAtLocation via reflection
            UFunction* TempFunc = BiomeSys->FindFunction(TEXT("GetTemperatureAtLocation"));
            if (TempFunc)
            {
                struct { FVector Loc; float RetVal; } TempParams;
                TempParams.Loc = Location;
                TempParams.RetVal = 24.0f;
                BiomeSys->ProcessEvent(TempFunc, &TempParams);
                CurrentBiomeTemperature = TempParams.RetVal;
            }

            // Call GetHazardLevelAtLocation via reflection
            UFunction* HazardFunc = BiomeSys->FindFunction(TEXT("GetHazardLevelAtLocation"));
            if (HazardFunc)
            {
                struct { FVector Loc; float RetVal; } HazardParams;
                HazardParams.Loc = Location;
                HazardParams.RetVal = 0.2f;
                BiomeSys->ProcessEvent(HazardFunc, &HazardParams);
                CurrentBiomeHazardLevel = HazardParams.RetVal;
            }
        }
    }
}

// ----------------------------------------------------------------
// Input Setup
// ----------------------------------------------------------------
void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight",   this, &ATranspersonalCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn",        this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp",      this, &APawn::AddControllerPitchInput);

    PlayerInputComponent->BindAction("Jump",  IE_Pressed,  this, &ATranspersonalCharacter::StartJump);
    PlayerInputComponent->BindAction("Jump",  IE_Released, this, &ATranspersonalCharacter::StopJump);
    PlayerInputComponent->BindAction("Sprint", IE_Pressed,  this, &ATranspersonalCharacter::StartRun);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATranspersonalCharacter::StopRun);
}

// ----------------------------------------------------------------
// Movement handlers
// ----------------------------------------------------------------
void ATranspersonalCharacter::MoveForward(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ATranspersonalCharacter::MoveRight(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void ATranspersonalCharacter::StartRun()
{
    if (CurrentStamina > 5.0f)
    {
        bIsRunning = true;
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
    }
}

void ATranspersonalCharacter::StopRun()
{
    bIsRunning = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ATranspersonalCharacter::StartJump()
{
    if (CurrentStamina > 10.0f)
    {
        Jump();
        if (SurvivalComp)
        {
            SurvivalComp->SetStamina(FMath::Max(0.0f, CurrentStamina - 10.0f));
        }
    }
}

void ATranspersonalCharacter::StopJump()
{
    StopJumping();
}
