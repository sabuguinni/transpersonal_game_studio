#include "SurvivalComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "World/BiomeManager.h"

// ─────────────────────────────────────────────────────────────────────────────
USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f; // We manage our own interval

    // Default stat configurations
    Health.Current            = 100.f;
    Health.Max                = 100.f;
    Health.DecayRatePerSecond = 0.0f;   // Health doesn't decay passively
    Health.CriticalThreshold  = 20.f;

    Hunger.Current            = 100.f;
    Hunger.Max                = 100.f;
    Hunger.DecayRatePerSecond = 0.8f;   // ~2 min to deplete at rest
    Hunger.CriticalThreshold  = 15.f;

    Thirst.Current            = 100.f;
    Thirst.Max                = 100.f;
    Thirst.DecayRatePerSecond = 1.2f;   // Thirst depletes faster than hunger
    Thirst.CriticalThreshold  = 15.f;

    Stamina.Current            = 100.f;
    Stamina.Max                = 100.f;
    Stamina.DecayRatePerSecond = 0.0f;  // Managed separately via sprint logic
    Stamina.CriticalThreshold  = 10.f;

    Temperature.Current            = 37.0f; // Normal body temp in Celsius
    Temperature.Max                = 42.0f; // Lethal upper bound
    Temperature.DecayRatePerSecond = 0.0f;  // Biome-driven
    Temperature.CriticalThreshold  = 34.0f; // Hypothermia threshold

    Fear.Current            = 0.f;
    Fear.Max                = 100.f;
    Fear.DecayRatePerSecond = -2.0f;    // Fear naturally decreases (negative = regen)
    Fear.CriticalThreshold  = 80.f;
}

// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Try to find BiomeManager in the world and register for biome change events
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> FoundManagers;
        UGameplayStatics::GetAllActorsOfClass(World, ABiomeManager::StaticClass(), FoundManagers);
        if (FoundManagers.Num() > 0)
        {
            ABiomeManager* BiomeMgr = Cast<ABiomeManager>(FoundManagers[0]);
            if (BiomeMgr)
            {
                // Bind to biome transition event
                BiomeMgr->OnPlayerEnteredBiome.AddDynamic(this, &USurvivalComponent::SetCurrentBiome);
                UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: Bound to BiomeManager OnPlayerEnteredBiome"));
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead) return;

    AccumulatedTime += DeltaTime;

    // ── Stamina: always ticked (sprint drain / regen) ─────────────────────
    if (bIsSprinting)
    {
        float Drain = SprintStaminaDrain * DeltaTime;
        Stamina.Current = FMath::Max(0.f, Stamina.Current - Drain);
        if (Stamina.IsDepleted())
        {
            SetSprinting(false);
        }
    }
    else
    {
        // Regen stamina when not sprinting
        float Regen = StaminaRegenRate * DeltaTime;
        Stamina.Current = FMath::Min(Stamina.Max, Stamina.Current + Regen);
    }

    // ── Survival tick at configured interval ─────────────────────────────
    if (AccumulatedTime < TickInterval) return;
    float TickDelta = AccumulatedTime;
    AccumulatedTime = 0.f;

    // Apply biome modifiers to decay rates
    float TempMod     = BiomeTempModifier;
    float HumidityMod = BiomeHumidityModifier;

    // Tick hunger and thirst with biome multipliers
    TickStat(Hunger,  ECore_SurvivalStat::Hunger,  TickDelta, TempMod);
    TickStat(Thirst,  ECore_SurvivalStat::Thirst,  TickDelta, TempMod * HumidityMod);
    TickStat(Fear,    ECore_SurvivalStat::Fear,     TickDelta, 1.0f);

    // Starvation: deal health damage when hunger is depleted
    if (Hunger.IsDepleted())
    {
        float StarveDmg = StarvationDamageRate * TickDelta;
        Health.Current = FMath::Max(0.f, Health.Current - StarveDmg);
        OnStatChanged.Broadcast(ECore_SurvivalStat::Health, Health.Current);
    }

    // Dehydration: deal health damage when thirst is depleted
    if (Thirst.IsDepleted())
    {
        float DehydDmg = DehydrationDamageRate * TickDelta;
        Health.Current = FMath::Max(0.f, Health.Current - DehydDmg);
        OnStatChanged.Broadcast(ECore_SurvivalStat::Health, Health.Current);
    }

    // Check critical thresholds
    if (Health.IsCritical())  HandleStatCritical(ECore_SurvivalStat::Health);
    if (Hunger.IsCritical())  HandleStatCritical(ECore_SurvivalStat::Hunger);
    if (Thirst.IsCritical())  HandleStatCritical(ECore_SurvivalStat::Thirst);
    if (Fear.IsCritical())    HandleStatCritical(ECore_SurvivalStat::Fear);

    // Death check
    if (Health.IsDepleted())
    {
        HandleDeath();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Stat accessors
// ─────────────────────────────────────────────────────────────────────────────
float USurvivalComponent::GetStat(ECore_SurvivalStat Stat) const
{
    return GetStatConstRef(Stat).Current;
}

float USurvivalComponent::GetStatNormalized(ECore_SurvivalStat Stat) const
{
    return GetStatConstRef(Stat).GetNormalized();
}

bool USurvivalComponent::IsStatCritical(ECore_SurvivalStat Stat) const
{
    return GetStatConstRef(Stat).IsCritical();
}

// ─────────────────────────────────────────────────────────────────────────────
// Stat modifiers
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::ApplyDamage(float Amount)
{
    if (bIsDead || Amount <= 0.f) return;
    Health.Current = FMath::Max(0.f, Health.Current - Amount);
    OnStatChanged.Broadcast(ECore_SurvivalStat::Health, Health.Current);
    if (Health.IsDepleted()) HandleDeath();
    else if (Health.IsCritical()) HandleStatCritical(ECore_SurvivalStat::Health);
}

void USurvivalComponent::Heal(float Amount)
{
    if (bIsDead || Amount <= 0.f) return;
    Health.Current = FMath::Min(Health.Max, Health.Current + Amount);
    OnStatChanged.Broadcast(ECore_SurvivalStat::Health, Health.Current);
}

void USurvivalComponent::ConsumeFood(float NutritionValue)
{
    if (NutritionValue <= 0.f) return;
    Hunger.Current = FMath::Min(Hunger.Max, Hunger.Current + NutritionValue);
    OnStatChanged.Broadcast(ECore_SurvivalStat::Hunger, Hunger.Current);
}

void USurvivalComponent::DrinkWater(float HydrationValue)
{
    if (HydrationValue <= 0.f) return;
    Thirst.Current = FMath::Min(Thirst.Max, Thirst.Current + HydrationValue);
    OnStatChanged.Broadcast(ECore_SurvivalStat::Thirst, Thirst.Current);
}

void USurvivalComponent::RestoreStamina(float Amount)
{
    if (Amount <= 0.f) return;
    Stamina.Current = FMath::Min(Stamina.Max, Stamina.Current + Amount);
    OnStatChanged.Broadcast(ECore_SurvivalStat::Stamina, Stamina.Current);
}

void USurvivalComponent::AddFear(float Amount)
{
    if (Amount <= 0.f) return;
    Fear.Current = FMath::Min(Fear.Max, Fear.Current + Amount);
    OnStatChanged.Broadcast(ECore_SurvivalStat::Fear, Fear.Current);
    if (Fear.IsCritical()) HandleStatCritical(ECore_SurvivalStat::Fear);
}

void USurvivalComponent::ReduceFear(float Amount)
{
    if (Amount <= 0.f) return;
    Fear.Current = FMath::Max(0.f, Fear.Current - Amount);
    OnStatChanged.Broadcast(ECore_SurvivalStat::Fear, Fear.Current);
}

// ─────────────────────────────────────────────────────────────────────────────
// Biome integration
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::ApplyBiomeModifiers(const FEng_BiomeConfig& BiomeConfig)
{
    // Temperature modifier: hot biomes increase hunger/thirst decay
    // Normalized: 0.5 (cold) to 2.0 (volcanic)
    float NormalizedTemp = FMath::GetMappedRangeValueClamped(
        FVector2D(-20.f, 50.f),
        FVector2D(0.5f, 2.0f),
        BiomeConfig.AmbientTemperatureCelsius
    );
    BiomeTempModifier = NormalizedTemp;

    // Humidity modifier: low humidity increases thirst decay
    // Normalized: 2.0 (desert/volcanic) to 0.7 (swamp/coastal)
    float NormalizedHumidity = FMath::GetMappedRangeValueClamped(
        FVector2D(0.f, 1.f),
        FVector2D(2.0f, 0.7f),
        BiomeConfig.HumidityFactor
    );
    BiomeHumidityModifier = NormalizedHumidity;

    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: Biome modifiers updated — TempMod=%.2f, HumidityMod=%.2f"),
           BiomeTempModifier, BiomeHumidityModifier);
}

void USurvivalComponent::SetCurrentBiome(EEng_BiomeType NewBiome)
{
    CurrentBiome = NewBiome;
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: Entered biome %d"), (int32)NewBiome);
}

// ─────────────────────────────────────────────────────────────────────────────
// Sprint / stamina
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::SetSprinting(bool bSprinting)
{
    if (bSprinting && !CanSprint()) return;

    bIsSprinting = bSprinting;

    // Adjust character movement speed
    AActor* Owner = GetOwner();
    if (Owner)
    {
        ACharacter* Char = Cast<ACharacter>(Owner);
        if (Char && Char->GetCharacterMovement())
        {
            Char->GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? 700.f : 400.f;
        }
    }
}

bool USurvivalComponent::CanSprint() const
{
    return !bIsDead && Stamina.Current >= SprintMinStamina;
}

// ─────────────────────────────────────────────────────────────────────────────
// Internal helpers
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::TickStat(FCore_StatValue& Stat, ECore_SurvivalStat StatType,
                                   float DeltaTime, float DecayMultiplier)
{
    if (Stat.DecayRatePerSecond == 0.f) return;

    float Delta = Stat.DecayRatePerSecond * DeltaTime * DecayMultiplier;
    float OldValue = Stat.Current;
    Stat.Current = FMath::Clamp(Stat.Current - Delta, 0.f, Stat.Max);

    if (!FMath::IsNearlyEqual(OldValue, Stat.Current, 0.01f))
    {
        OnStatChanged.Broadcast(StatType, Stat.Current);
    }
}

void USurvivalComponent::HandleStatCritical(ECore_SurvivalStat Stat)
{
    OnStatCritical.Broadcast(Stat);
}

void USurvivalComponent::HandleDeath()
{
    if (bIsDead) return;
    bIsDead = true;
    bIsSprinting = false;
    UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent: Character has died!"));
    OnCharacterDied.Broadcast();
}

FCore_StatValue& USurvivalComponent::GetStatRef(ECore_SurvivalStat Stat)
{
    switch (Stat)
    {
        case ECore_SurvivalStat::Health:      return Health;
        case ECore_SurvivalStat::Hunger:      return Hunger;
        case ECore_SurvivalStat::Thirst:      return Thirst;
        case ECore_SurvivalStat::Stamina:     return Stamina;
        case ECore_SurvivalStat::Temperature: return Temperature;
        case ECore_SurvivalStat::Fear:        return Fear;
        default:                              return Health;
    }
}

const FCore_StatValue& USurvivalComponent::GetStatConstRef(ECore_SurvivalStat Stat) const
{
    switch (Stat)
    {
        case ECore_SurvivalStat::Health:      return Health;
        case ECore_SurvivalStat::Hunger:      return Hunger;
        case ECore_SurvivalStat::Thirst:      return Thirst;
        case ECore_SurvivalStat::Stamina:     return Stamina;
        case ECore_SurvivalStat::Temperature: return Temperature;
        case ECore_SurvivalStat::Fear:        return Fear;
        default:                              return Health;
    }
}
