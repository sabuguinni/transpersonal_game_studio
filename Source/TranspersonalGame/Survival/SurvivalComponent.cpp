// SurvivalComponent.cpp
// Performance Optimizer — Agent #04 | PROD_CYCLE_AUTO_20260624_003
// Implements survival stat drain/recovery logic for the prehistoric survival game.
// No spiritual/mystical content — pure biological survival mechanics.

#include "SurvivalComponent.h"
#include "TranspersonalCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz — sufficient for survival stats

    // Default stat values
    Health       = 100.0f;
    MaxHealth    = 100.0f;
    Hunger       = 100.0f;
    MaxHunger    = 100.0f;
    Thirst       = 100.0f;
    MaxThirst    = 100.0f;
    Temperature  = 37.0f;   // Celsius — human core body temperature
    Stamina      = 100.0f;
    MaxStamina   = 100.0f;
    Fear         = 0.0f;
    MaxFear      = 100.0f;

    // Drain rates (units per second)
    HungerDrainRate      = 0.5f;   // ~200s to empty
    ThirstDrainRate      = 0.8f;   // ~125s to empty
    StaminaSprintDrain   = 10.0f;  // per second while sprinting
    StaminaRestoreRate   = 5.0f;   // per second while not sprinting
    FearDecayRate        = 2.0f;   // per second when no predator nearby
    FearBuildRate        = 15.0f;  // per second when predator is close
    PredatorDetectRadius = 1500.0f;

    // Damage from starvation / dehydration
    StarvationDamageRate    = 2.0f;  // HP/s when hunger == 0
    DehydrationDamageRate   = 3.0f;  // HP/s when thirst == 0
    HypothermiaDamageRate   = 1.5f;  // HP/s when temp < 32°C
    HyperthermaDamageRate   = 1.5f;  // HP/s when temp > 42°C

    // Temperature comfort range
    ComfortTempMin = 15.0f;
    ComfortTempMax = 38.0f;
    AmbientTemperature = 28.0f;   // default warm prehistoric climate
    TempChangeRate = 0.2f;

    bIsSprinting = false;
    bIsDead      = false;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache owner character
    OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead) return;

    UpdateHunger(DeltaTime);
    UpdateThirst(DeltaTime);
    UpdateTemperature(DeltaTime);
    UpdateStamina(DeltaTime);
    UpdateFear(DeltaTime);
    ApplyDamageFromStats(DeltaTime);
}

// ---------------------------------------------------------------------------
// Hunger
// ---------------------------------------------------------------------------
void USurvivalComponent::UpdateHunger(float DeltaTime)
{
    if (Hunger <= 0.0f) return;

    Hunger = FMath::Max(0.0f, Hunger - HungerDrainRate * DeltaTime);

    // Broadcast when crossing thresholds
    if (Hunger < 20.0f)
    {
        OnHungerLow.Broadcast(Hunger);
    }
}

void USurvivalComponent::ConsumeFood(float Amount)
{
    Hunger = FMath::Min(MaxHunger, Hunger + Amount);
}

// ---------------------------------------------------------------------------
// Thirst
// ---------------------------------------------------------------------------
void USurvivalComponent::UpdateThirst(float DeltaTime)
{
    if (Thirst <= 0.0f) return;

    Thirst = FMath::Max(0.0f, Thirst - ThirstDrainRate * DeltaTime);

    if (Thirst < 20.0f)
    {
        OnThirstLow.Broadcast(Thirst);
    }
}

void USurvivalComponent::DrinkWater(float Amount)
{
    Thirst = FMath::Min(MaxThirst, Thirst + Amount);
}

// ---------------------------------------------------------------------------
// Temperature — biome-driven, moves toward ambient temperature
// ---------------------------------------------------------------------------
void USurvivalComponent::UpdateTemperature(float DeltaTime)
{
    // Gradually move body temperature toward ambient
    float Delta = AmbientTemperature - Temperature;
    Temperature += Delta * TempChangeRate * DeltaTime;
    Temperature = FMath::Clamp(Temperature, -10.0f, 50.0f);
}

void USurvivalComponent::SetAmbientTemperature(float NewAmbientTemp)
{
    AmbientTemperature = FMath::Clamp(NewAmbientTemp, -20.0f, 60.0f);
}

// ---------------------------------------------------------------------------
// Stamina — sprint drain / rest recovery
// ---------------------------------------------------------------------------
void USurvivalComponent::UpdateStamina(float DeltaTime)
{
    if (OwnerCharacter)
    {
        UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
        if (MoveComp)
        {
            bIsSprinting = (MoveComp->MaxWalkSpeed > 400.0f && MoveComp->Velocity.SizeSquared() > 10000.0f);
        }
    }

    if (bIsSprinting)
    {
        Stamina = FMath::Max(0.0f, Stamina - StaminaSprintDrain * DeltaTime);
        if (Stamina <= 0.0f && OwnerCharacter)
        {
            // Force walk when exhausted
            OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = 300.0f;
            OnStaminaExhausted.Broadcast();
        }
    }
    else
    {
        // Recover stamina when not sprinting
        Stamina = FMath::Min(MaxStamina, Stamina + StaminaRestoreRate * DeltaTime);
    }
}

// ---------------------------------------------------------------------------
// Fear — increases near predators, decays when safe
// ---------------------------------------------------------------------------
void USurvivalComponent::UpdateFear(float DeltaTime)
{
    bool bPredatorNearby = false;

    if (OwnerCharacter)
    {
        UWorld* World = OwnerCharacter->GetWorld();
        if (World)
        {
            // Sphere overlap to detect dinosaur pawns
            TArray<FOverlapResult> Overlaps;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(OwnerCharacter);

            World->OverlapMultiByChannel(
                Overlaps,
                OwnerCharacter->GetActorLocation(),
                FQuat::Identity,
                ECC_Pawn,
                FCollisionShape::MakeSphere(PredatorDetectRadius),
                QueryParams
            );

            for (const FOverlapResult& Overlap : Overlaps)
            {
                AActor* Other = Overlap.GetActor();
                if (Other && Other->ActorHasTag(FName("Predator")))
                {
                    bPredatorNearby = true;
                    break;
                }
            }
        }
    }

    if (bPredatorNearby)
    {
        Fear = FMath::Min(MaxFear, Fear + FearBuildRate * DeltaTime);
        if (Fear >= MaxFear)
        {
            OnPanicTriggered.Broadcast();
        }
    }
    else
    {
        Fear = FMath::Max(0.0f, Fear - FearDecayRate * DeltaTime);
    }
}

void USurvivalComponent::AddFear(float Amount)
{
    Fear = FMath::Clamp(Fear + Amount, 0.0f, MaxFear);
}

// ---------------------------------------------------------------------------
// Damage from critical stat failures
// ---------------------------------------------------------------------------
void USurvivalComponent::ApplyDamageFromStats(float DeltaTime)
{
    float TotalDamage = 0.0f;

    // Starvation
    if (Hunger <= 0.0f)
    {
        TotalDamage += StarvationDamageRate * DeltaTime;
    }

    // Dehydration
    if (Thirst <= 0.0f)
    {
        TotalDamage += DehydrationDamageRate * DeltaTime;
    }

    // Hypothermia
    if (Temperature < ComfortTempMin)
    {
        float Severity = (ComfortTempMin - Temperature) / ComfortTempMin;
        TotalDamage += HypothermiaDamageRate * Severity * DeltaTime;
    }

    // Hyperthermia
    if (Temperature > ComfortTempMax)
    {
        float Severity = (Temperature - ComfortTempMax) / (50.0f - ComfortTempMax);
        TotalDamage += HyperthermaDamageRate * Severity * DeltaTime;
    }

    if (TotalDamage > 0.0f)
    {
        ApplyHealthDamage(TotalDamage);
    }
}

void USurvivalComponent::ApplyHealthDamage(float Amount)
{
    Health = FMath::Max(0.0f, Health - Amount);

    if (Health <= 0.0f && !bIsDead)
    {
        bIsDead = true;
        OnDeath.Broadcast();
    }
}

void USurvivalComponent::HealHealth(float Amount)
{
    if (bIsDead) return;
    Health = FMath::Min(MaxHealth, Health + Amount);
}

// ---------------------------------------------------------------------------
// Stat accessors (Blueprint-callable)
// ---------------------------------------------------------------------------
float USurvivalComponent::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f;
}

float USurvivalComponent::GetHungerPercent() const
{
    return (MaxHunger > 0.0f) ? (Hunger / MaxHunger) : 0.0f;
}

float USurvivalComponent::GetThirstPercent() const
{
    return (MaxThirst > 0.0f) ? (Thirst / MaxThirst) : 0.0f;
}

float USurvivalComponent::GetStaminaPercent() const
{
    return (MaxStamina > 0.0f) ? (Stamina / MaxStamina) : 0.0f;
}

float USurvivalComponent::GetFearPercent() const
{
    return (MaxFear > 0.0f) ? (Fear / MaxFear) : 0.0f;
}

bool USurvivalComponent::IsStarving() const  { return Hunger <= 0.0f; }
bool USurvivalComponent::IsDehydrated() const { return Thirst <= 0.0f; }
bool USurvivalComponent::IsExhausted() const  { return Stamina <= 0.0f; }
bool USurvivalComponent::IsInPanic() const    { return Fear >= MaxFear; }
bool USurvivalComponent::IsDead() const       { return bIsDead; }
