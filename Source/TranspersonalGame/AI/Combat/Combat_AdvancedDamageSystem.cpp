#include "Combat_AdvancedDamageSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

UCombat_AdvancedDamageSystem::UCombat_AdvancedDamageSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for bleeding
    
    DamageReductionPercentage = 0.0f;
    bIsInvulnerable = false;
    BleedingDamagePerSecond = 5.0f;
    BleedingDuration = 10.0f;
    bIsBleeding = false;
    BleedingTimeRemaining = 0.0f;
}

void UCombat_AdvancedDamageSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDamageTypes();
}

void UCombat_AdvancedDamageSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsBleeding)
    {
        BleedingTimeRemaining -= DeltaTime;
        if (BleedingTimeRemaining <= 0.0f)
        {
            StopBleeding();
        }
    }
}

void UCombat_AdvancedDamageSystem::InitializeDamageTypes()
{
    // Initialize Physical damage
    FCombat_DamageData PhysicalDamage;
    PhysicalDamage.BaseDamage = 25.0f;
    PhysicalDamage.DamageVariance = 10.0f;
    PhysicalDamage.DamageType = ECombat_DamageType::Physical;
    PhysicalDamage.bCanCauseBleeding = false;
    PhysicalDamage.CriticalHitChance = 0.1f;
    PhysicalDamage.CriticalMultiplier = 1.5f;
    DamageTypeData.Add(ECombat_DamageType::Physical, PhysicalDamage);

    // Initialize Bite damage
    FCombat_DamageData BiteDamage;
    BiteDamage.BaseDamage = 45.0f;
    BiteDamage.DamageVariance = 15.0f;
    BiteDamage.DamageType = ECombat_DamageType::Bite;
    BiteDamage.bCanCauseBleeding = true;
    BiteDamage.CriticalHitChance = 0.15f;
    BiteDamage.CriticalMultiplier = 2.0f;
    DamageTypeData.Add(ECombat_DamageType::Bite, BiteDamage);

    // Initialize Claw damage
    FCombat_DamageData ClawDamage;
    ClawDamage.BaseDamage = 35.0f;
    ClawDamage.DamageVariance = 12.0f;
    ClawDamage.DamageType = ECombat_DamageType::Claw;
    ClawDamage.bCanCauseBleeding = true;
    ClawDamage.CriticalHitChance = 0.2f;
    ClawDamage.CriticalMultiplier = 1.8f;
    DamageTypeData.Add(ECombat_DamageType::Claw, ClawDamage);

    // Initialize Crush damage
    FCombat_DamageData CrushDamage;
    CrushDamage.BaseDamage = 80.0f;
    CrushDamage.DamageVariance = 20.0f;
    CrushDamage.DamageType = ECombat_DamageType::Crush;
    CrushDamage.bCanCauseBleeding = false;
    CrushDamage.CriticalHitChance = 0.05f;
    CrushDamage.CriticalMultiplier = 2.5f;
    DamageTypeData.Add(ECombat_DamageType::Crush, CrushDamage);

    // Initialize Fall damage
    FCombat_DamageData FallDamage;
    FallDamage.BaseDamage = 20.0f;
    FallDamage.DamageVariance = 5.0f;
    FallDamage.DamageType = ECombat_DamageType::Fall;
    FallDamage.bCanCauseBleeding = false;
    FallDamage.CriticalHitChance = 0.0f;
    FallDamage.CriticalMultiplier = 1.0f;
    DamageTypeData.Add(ECombat_DamageType::Fall, FallDamage);

    // Initialize Environmental damage
    FCombat_DamageData EnvironmentalDamage;
    EnvironmentalDamage.BaseDamage = 15.0f;
    EnvironmentalDamage.DamageVariance = 8.0f;
    EnvironmentalDamage.DamageType = ECombat_DamageType::Environmental;
    EnvironmentalDamage.bCanCauseBleeding = false;
    EnvironmentalDamage.CriticalHitChance = 0.0f;
    EnvironmentalDamage.CriticalMultiplier = 1.0f;
    DamageTypeData.Add(ECombat_DamageType::Environmental, EnvironmentalDamage);
}

FCombat_DamageResult UCombat_AdvancedDamageSystem::ApplyDamage(const FCombat_DamageData& DamageData, AActor* DamageSource)
{
    FCombat_DamageResult Result;
    
    if (bIsInvulnerable)
    {
        Result.FinalDamage = 0.0f;
        Result.bWasCritical = false;
        Result.bCausedBleeding = false;
        Result.DamageType = DamageData.DamageType;
        return Result;
    }

    // Calculate base damage with variance
    float BaseDamage = CalculateRandomDamage(DamageData);
    
    // Check for critical hit
    bool bIsCritical = RollForCritical(DamageData.CriticalHitChance);
    if (bIsCritical)
    {
        BaseDamage *= DamageData.CriticalMultiplier;
        OnCriticalHit(Result);
    }

    // Apply damage reduction
    float FinalDamage = BaseDamage * (1.0f - (DamageReductionPercentage / 100.0f));
    FinalDamage = FMath::Max(0.0f, FinalDamage);

    // Check for bleeding
    bool bCausedBleeding = false;
    if (DamageData.bCanCauseBleeding && FinalDamage > 0.0f)
    {
        float BleedingChance = FMath::Clamp(FinalDamage / 100.0f, 0.1f, 0.8f);
        if (FMath::RandRange(0.0f, 1.0f) <= BleedingChance)
        {
            StartBleeding(BleedingDuration);
            bCausedBleeding = true;
        }
    }

    // Fill result structure
    Result.FinalDamage = FinalDamage;
    Result.bWasCritical = bIsCritical;
    Result.bCausedBleeding = bCausedBleeding;
    Result.DamageType = DamageData.DamageType;

    // Trigger damage applied event
    OnDamageApplied(Result, DamageSource);

    return Result;
}

void UCombat_AdvancedDamageSystem::SetDamageReduction(float ReductionPercentage)
{
    DamageReductionPercentage = FMath::Clamp(ReductionPercentage, 0.0f, 95.0f);
}

void UCombat_AdvancedDamageSystem::SetInvulnerability(bool bInvulnerable)
{
    bIsInvulnerable = bInvulnerable;
}

void UCombat_AdvancedDamageSystem::StartBleeding(float Duration)
{
    if (!bIsBleeding)
    {
        bIsBleeding = true;
        BleedingTimeRemaining = Duration;
        OnBleedingStarted();
        
        // Start bleeding timer
        GetWorld()->GetTimerManager().SetTimer(
            BleedingTimerHandle,
            this,
            &UCombat_AdvancedDamageSystem::ProcessBleeding,
            1.0f,
            true
        );
    }
    else
    {
        // Extend bleeding duration
        BleedingTimeRemaining = FMath::Max(BleedingTimeRemaining, Duration);
    }
}

void UCombat_AdvancedDamageSystem::StopBleeding()
{
    if (bIsBleeding)
    {
        bIsBleeding = false;
        BleedingTimeRemaining = 0.0f;
        GetWorld()->GetTimerManager().ClearTimer(BleedingTimerHandle);
        OnBleedingStopped();
    }
}

float UCombat_AdvancedDamageSystem::CalculateFallDamage(float FallHeight)
{
    // Fall damage increases exponentially with height
    float BaseFallDamage = FMath::Pow(FallHeight / 100.0f, 1.5f) * 25.0f;
    return FMath::Max(0.0f, BaseFallDamage);
}

FCombat_DamageData UCombat_AdvancedDamageSystem::GetDamageDataForType(ECombat_DamageType DamageType)
{
    if (DamageTypeData.Contains(DamageType))
    {
        return DamageTypeData[DamageType];
    }
    
    // Return default physical damage if type not found
    FCombat_DamageData DefaultDamage;
    return DefaultDamage;
}

void UCombat_AdvancedDamageSystem::ProcessBleeding()
{
    if (bIsBleeding && BleedingTimeRemaining > 0.0f)
    {
        FCombat_DamageData BleedingDamageData;
        BleedingDamageData.BaseDamage = BleedingDamagePerSecond;
        BleedingDamageData.DamageVariance = 2.0f;
        BleedingDamageData.DamageType = ECombat_DamageType::Environmental;
        BleedingDamageData.bCanCauseBleeding = false;
        BleedingDamageData.CriticalHitChance = 0.0f;
        
        ApplyDamage(BleedingDamageData, nullptr);
    }
}

float UCombat_AdvancedDamageSystem::CalculateRandomDamage(const FCombat_DamageData& DamageData)
{
    float MinDamage = DamageData.BaseDamage - DamageData.DamageVariance;
    float MaxDamage = DamageData.BaseDamage + DamageData.DamageVariance;
    return FMath::RandRange(MinDamage, MaxDamage);
}

bool UCombat_AdvancedDamageSystem::RollForCritical(float CriticalChance)
{
    return FMath::RandRange(0.0f, 1.0f) <= CriticalChance;
}