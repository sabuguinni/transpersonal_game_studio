#include "Eng_SurvivalSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

void UEng_SurvivalSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize default survival configuration
    SurvivalConfig.HealthDecayRate = 0.1f;
    SurvivalConfig.HungerDecayRate = 1.0f;
    SurvivalConfig.ThirstDecayRate = 1.5f;
    SurvivalConfig.StaminaRegenRate = 5.0f;
    SurvivalConfig.FearDecayRate = 2.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_SurvivalSystemManager initialized"));
}

void UEng_SurvivalSystemManager::Deinitialize()
{
    ActorSurvivalStats.Empty();
    Super::Deinitialize();
}

void UEng_SurvivalSystemManager::UpdateSurvivalStats(AActor* Actor, float DeltaTime)
{
    if (!Actor)
    {
        return;
    }

    if (!ActorSurvivalStats.Contains(Actor))
    {
        InitializeActorStats(Actor);
    }

    FEng_SurvivalStats& Stats = ActorSurvivalStats[Actor];

    // Update hunger and thirst
    Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - (SurvivalConfig.HungerDecayRate * DeltaTime));
    Stats.Thirst = FMath::Max(0.0f, Stats.Thirst - (SurvivalConfig.ThirstDecayRate * DeltaTime));

    // Regenerate stamina
    Stats.Stamina = FMath::Min(100.0f, Stats.Stamina + (SurvivalConfig.StaminaRegenRate * DeltaTime));

    // Decay fear over time
    Stats.Fear = FMath::Max(0.0f, Stats.Fear - (SurvivalConfig.FearDecayRate * DeltaTime));

    // Process health effects based on other stats
    ProcessHealthEffects(Actor, Stats, DeltaTime);

    // Process temperature effects
    ProcessTemperatureEffects(Actor, Stats, DeltaTime);
}

FEng_SurvivalStats UEng_SurvivalSystemManager::GetSurvivalStats(AActor* Actor)
{
    if (!Actor || !ActorSurvivalStats.Contains(Actor))
    {
        return FEng_SurvivalStats();
    }

    return ActorSurvivalStats[Actor];
}

void UEng_SurvivalSystemManager::SetSurvivalStats(AActor* Actor, const FEng_SurvivalStats& NewStats)
{
    if (!Actor)
    {
        return;
    }

    ActorSurvivalStats.Add(Actor, NewStats);
}

bool UEng_SurvivalSystemManager::IsActorAlive(AActor* Actor)
{
    if (!Actor || !ActorSurvivalStats.Contains(Actor))
    {
        return false;
    }

    return ActorSurvivalStats[Actor].Health > 0.0f;
}

void UEng_SurvivalSystemManager::ApplyDamage(AActor* Actor, float Damage)
{
    if (!Actor)
    {
        return;
    }

    if (!ActorSurvivalStats.Contains(Actor))
    {
        InitializeActorStats(Actor);
    }

    FEng_SurvivalStats& Stats = ActorSurvivalStats[Actor];
    Stats.Health = FMath::Max(0.0f, Stats.Health - Damage);

    // Add fear when taking damage
    Stats.Fear = FMath::Min(100.0f, Stats.Fear + (Damage * 0.5f));
}

void UEng_SurvivalSystemManager::ApplyHealing(AActor* Actor, float Healing)
{
    if (!Actor)
    {
        return;
    }

    if (!ActorSurvivalStats.Contains(Actor))
    {
        InitializeActorStats(Actor);
    }

    FEng_SurvivalStats& Stats = ActorSurvivalStats[Actor];
    Stats.Health = FMath::Min(100.0f, Stats.Health + Healing);
}

void UEng_SurvivalSystemManager::ConsumeFood(AActor* Actor, float FoodValue)
{
    if (!Actor)
    {
        return;
    }

    if (!ActorSurvivalStats.Contains(Actor))
    {
        InitializeActorStats(Actor);
    }

    FEng_SurvivalStats& Stats = ActorSurvivalStats[Actor];
    Stats.Hunger = FMath::Min(100.0f, Stats.Hunger + FoodValue);
}

void UEng_SurvivalSystemManager::ConsumeDrink(AActor* Actor, float DrinkValue)
{
    if (!Actor)
    {
        return;
    }

    if (!ActorSurvivalStats.Contains(Actor))
    {
        InitializeActorStats(Actor);
    }

    FEng_SurvivalStats& Stats = ActorSurvivalStats[Actor];
    Stats.Thirst = FMath::Min(100.0f, Stats.Thirst + DrinkValue);
}

void UEng_SurvivalSystemManager::AddFear(AActor* Actor, float FearAmount)
{
    if (!Actor)
    {
        return;
    }

    if (!ActorSurvivalStats.Contains(Actor))
    {
        InitializeActorStats(Actor);
    }

    FEng_SurvivalStats& Stats = ActorSurvivalStats[Actor];
    Stats.Fear = FMath::Min(100.0f, Stats.Fear + FearAmount);
}

void UEng_SurvivalSystemManager::SetTemperature(AActor* Actor, float NewTemperature)
{
    if (!Actor)
    {
        return;
    }

    if (!ActorSurvivalStats.Contains(Actor))
    {
        InitializeActorStats(Actor);
    }

    FEng_SurvivalStats& Stats = ActorSurvivalStats[Actor];
    Stats.Temperature = NewTemperature;
}

void UEng_SurvivalSystemManager::InitializeActorStats(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    FEng_SurvivalStats NewStats;
    NewStats.Health = 100.0f;
    NewStats.Hunger = 100.0f;
    NewStats.Thirst = 100.0f;
    NewStats.Stamina = 100.0f;
    NewStats.Fear = 0.0f;
    NewStats.Temperature = 20.0f;

    ActorSurvivalStats.Add(Actor, NewStats);
}

void UEng_SurvivalSystemManager::ProcessHealthEffects(AActor* Actor, FEng_SurvivalStats& Stats, float DeltaTime)
{
    // Health decreases if hungry or thirsty
    if (Stats.Hunger < 20.0f || Stats.Thirst < 20.0f)
    {
        float HealthLoss = SurvivalConfig.HealthDecayRate * DeltaTime;
        if (Stats.Hunger < 10.0f) HealthLoss *= 2.0f;
        if (Stats.Thirst < 10.0f) HealthLoss *= 2.0f;
        
        Stats.Health = FMath::Max(0.0f, Stats.Health - HealthLoss);
    }

    // Extreme fear affects health
    if (Stats.Fear > 80.0f)
    {
        Stats.Health = FMath::Max(0.0f, Stats.Health - (SurvivalConfig.HealthDecayRate * 0.5f * DeltaTime));
    }
}

void UEng_SurvivalSystemManager::ProcessTemperatureEffects(AActor* Actor, FEng_SurvivalStats& Stats, float DeltaTime)
{
    // Temperature affects thirst and health
    if (Stats.Temperature > 35.0f) // Hot weather
    {
        Stats.Thirst = FMath::Max(0.0f, Stats.Thirst - (SurvivalConfig.ThirstDecayRate * 0.5f * DeltaTime));
    }
    else if (Stats.Temperature < 5.0f) // Cold weather
    {
        Stats.Health = FMath::Max(0.0f, Stats.Health - (SurvivalConfig.HealthDecayRate * 0.3f * DeltaTime));
        Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - (SurvivalConfig.HungerDecayRate * 0.3f * DeltaTime));
    }
}