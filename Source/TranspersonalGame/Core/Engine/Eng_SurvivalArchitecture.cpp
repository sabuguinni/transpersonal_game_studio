#include "Eng_SurvivalArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UEng_SurvivalArchitecture::UEng_SurvivalArchitecture()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    // Initialize default survival stats
    CurrentStats.Health = 100.0f;
    CurrentStats.MaxHealth = 100.0f;
    CurrentStats.Hunger = 100.0f;
    CurrentStats.MaxHunger = 100.0f;
    CurrentStats.Thirst = 100.0f;
    CurrentStats.MaxThirst = 100.0f;
    CurrentStats.Stamina = 100.0f;
    CurrentStats.MaxStamina = 100.0f;
    CurrentStats.Temperature = 37.0f;
    CurrentStats.Fear = 0.0f;
    CurrentStats.MaxFear = 100.0f;
    
    // Initialize environmental factors
    EnvironmentalState.AmbientTemperature = 25.0f;
    EnvironmentalState.Humidity = 50.0f;
    EnvironmentalState.WindSpeed = 0.0f;
    EnvironmentalState.bIsRaining = false;
    EnvironmentalState.TimeOfDay = 12.0f;
    EnvironmentalState.DangerLevel = 0.0f;
    
    // Initialize architecture rules
    ArchitectureRules.HealthDecayRate = 0.1f;
    ArchitectureRules.HungerDecayRate = 1.0f;
    ArchitectureRules.ThirstDecayRate = 1.5f;
    ArchitectureRules.StaminaRegenRate = 5.0f;
    ArchitectureRules.TemperatureChangeRate = 2.0f;
    ArchitectureRules.FearDecayRate = 0.5f;
    ArchitectureRules.bEnableRealisticSurvival = true;
    ArchitectureRules.bEnableEnvironmentalEffects = true;
    ArchitectureRules.bEnableThreatSystem = true;
}

void UEng_SurvivalArchitecture::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeSurvivalSystems();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Survival Architecture initialized"));
}

void UEng_SurvivalArchitecture::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsInitialized && ArchitectureRules.bEnableRealisticSurvival)
    {
        UpdateSurvivalStats(DeltaTime);
        ProcessEnvironmentalEffects(DeltaTime);
        UpdateThreatAssessment();
    }
    
    LastUpdateTime += DeltaTime;
}

bool UEng_SurvivalArchitecture::ValidateSurvivalArchitecture()
{
    ArchitectureViolations.Empty();
    
    // Validate survival stats ranges
    if (CurrentStats.Health < 0.0f || CurrentStats.Health > CurrentStats.MaxHealth)
    {
        ArchitectureViolations.Add("Health stat out of valid range");
    }
    
    if (CurrentStats.Hunger < 0.0f || CurrentStats.Hunger > CurrentStats.MaxHunger)
    {
        ArchitectureViolations.Add("Hunger stat out of valid range");
    }
    
    if (CurrentStats.Thirst < 0.0f || CurrentStats.Thirst > CurrentStats.MaxThirst)
    {
        ArchitectureViolations.Add("Thirst stat out of valid range");
    }
    
    if (CurrentStats.Stamina < 0.0f || CurrentStats.Stamina > CurrentStats.MaxStamina)
    {
        ArchitectureViolations.Add("Stamina stat out of valid range");
    }
    
    if (CurrentStats.Fear < 0.0f || CurrentStats.Fear > CurrentStats.MaxFear)
    {
        ArchitectureViolations.Add("Fear stat out of valid range");
    }
    
    // Validate environmental factors
    if (EnvironmentalState.TimeOfDay < 0.0f || EnvironmentalState.TimeOfDay > 24.0f)
    {
        ArchitectureViolations.Add("Time of day out of valid range (0-24)");
    }
    
    if (EnvironmentalState.DangerLevel < 0.0f || EnvironmentalState.DangerLevel > 100.0f)
    {
        ArchitectureViolations.Add("Danger level out of valid range (0-100)");
    }
    
    // Validate architecture rules
    if (ArchitectureRules.HungerDecayRate <= 0.0f || ArchitectureRules.ThirstDecayRate <= 0.0f)
    {
        ArchitectureViolations.Add("Invalid decay rates - must be positive");
    }
    
    bool bIsValid = ArchitectureViolations.Num() == 0;
    
    if (bIsValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("Survival Architecture validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Survival Architecture validation FAILED - %d violations"), ArchitectureViolations.Num());
        for (const FString& Violation : ArchitectureViolations)
        {
            UE_LOG(LogTemp, Error, TEXT("- %s"), *Violation);
        }
    }
    
    return bIsValid;
}

void UEng_SurvivalArchitecture::InitializeSurvivalSystems()
{
    // Reset all systems to default state
    CurrentStats.Health = CurrentStats.MaxHealth;
    CurrentStats.Hunger = CurrentStats.MaxHunger;
    CurrentStats.Thirst = CurrentStats.MaxThirst;
    CurrentStats.Stamina = CurrentStats.MaxStamina;
    CurrentStats.Temperature = 37.0f;
    CurrentStats.Fear = 0.0f;
    
    // Clear environmental threats
    EnvironmentalState.NearbyThreats.Empty();
    EnvironmentalState.DangerLevel = 0.0f;
    
    // Validate the architecture
    bIsInitialized = ValidateSurvivalArchitecture();
    
    UE_LOG(LogTemp, Warning, TEXT("Survival systems initialized - Status: %s"), 
           bIsInitialized ? TEXT("SUCCESS") : TEXT("FAILED"));
}

void UEng_SurvivalArchitecture::UpdateSurvivalStats(float DeltaTime)
{
    if (!ArchitectureRules.bEnableRealisticSurvival) return;
    
    // Apply survival decay
    ProcessSurvivalDecay(DeltaTime);
    
    // Apply environmental effects
    ApplyEnvironmentalEffects(DeltaTime);
    
    // Validate stat ranges after updates
    ValidateStatRanges();
}

void UEng_SurvivalArchitecture::ProcessEnvironmentalEffects(float DeltaTime)
{
    if (!ArchitectureRules.bEnableEnvironmentalEffects) return;
    
    // Temperature effects
    float TemperatureEffect = CalculateTemperatureEffect();
    ModifyTemperature(TemperatureEffect * DeltaTime);
    
    // Weather effects
    float WeatherEffect = CalculateWeatherEffect();
    if (EnvironmentalState.bIsRaining)
    {
        ModifyThirst(-WeatherEffect * DeltaTime * 0.5f); // Rain reduces thirst slightly
        ModifyTemperature(-2.0f * DeltaTime); // Rain cools down
    }
    
    // Time of day effects
    if (EnvironmentalState.TimeOfDay < 6.0f || EnvironmentalState.TimeOfDay > 20.0f)
    {
        // Night time - increased fear, slower stamina regen
        ModifyFear(1.0f * DeltaTime);
        ArchitectureRules.StaminaRegenRate *= 0.8f;
    }
    else
    {
        // Day time - reduced fear, normal stamina regen
        ModifyFear(-0.5f * DeltaTime);
        ArchitectureRules.StaminaRegenRate = 5.0f;
    }
}

void UEng_SurvivalArchitecture::ModifyHealth(float Amount)
{
    CurrentStats.Health = FMath::Clamp(CurrentStats.Health + Amount, 0.0f, CurrentStats.MaxHealth);
}

void UEng_SurvivalArchitecture::ModifyHunger(float Amount)
{
    CurrentStats.Hunger = FMath::Clamp(CurrentStats.Hunger + Amount, 0.0f, CurrentStats.MaxHunger);
}

void UEng_SurvivalArchitecture::ModifyThirst(float Amount)
{
    CurrentStats.Thirst = FMath::Clamp(CurrentStats.Thirst + Amount, 0.0f, CurrentStats.MaxThirst);
}

void UEng_SurvivalArchitecture::ModifyStamina(float Amount)
{
    CurrentStats.Stamina = FMath::Clamp(CurrentStats.Stamina + Amount, 0.0f, CurrentStats.MaxStamina);
}

void UEng_SurvivalArchitecture::ModifyTemperature(float Amount)
{
    CurrentStats.Temperature = FMath::Clamp(CurrentStats.Temperature + Amount, 20.0f, 45.0f);
}

void UEng_SurvivalArchitecture::ModifyFear(float Amount)
{
    CurrentStats.Fear = FMath::Clamp(CurrentStats.Fear + Amount, 0.0f, CurrentStats.MaxFear);
}

void UEng_SurvivalArchitecture::UpdateEnvironmentalFactors(const FEng_EnvironmentalFactors& NewFactors)
{
    EnvironmentalState = NewFactors;
    
    // Recalculate danger level based on threats
    EnvironmentalState.DangerLevel = FMath::Min(EnvironmentalState.NearbyThreats.Num() * 20.0f, 100.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Environmental factors updated - Danger Level: %.1f"), 
           EnvironmentalState.DangerLevel);
}

float UEng_SurvivalArchitecture::CalculateTemperatureEffect() const
{
    float TargetTemp = EnvironmentalState.AmbientTemperature;
    float CurrentTemp = CurrentStats.Temperature;
    
    // Calculate temperature change rate based on difference
    float TempDifference = TargetTemp - CurrentTemp;
    return TempDifference * ArchitectureRules.TemperatureChangeRate * 0.1f;
}

float UEng_SurvivalArchitecture::CalculateWeatherEffect() const
{
    float Effect = 1.0f;
    
    if (EnvironmentalState.bIsRaining)
    {
        Effect += 0.5f; // Rain increases environmental stress
    }
    
    if (EnvironmentalState.WindSpeed > 10.0f)
    {
        Effect += 0.3f; // High wind increases environmental stress
    }
    
    return Effect;
}

void UEng_SurvivalArchitecture::AddThreat(const FString& ThreatName)
{
    if (!EnvironmentalState.NearbyThreats.Contains(ThreatName))
    {
        EnvironmentalState.NearbyThreats.Add(ThreatName);
        EnvironmentalState.DangerLevel = FMath::Min(EnvironmentalState.NearbyThreats.Num() * 20.0f, 100.0f);
        
        // Increase fear based on threat
        ModifyFear(15.0f);
        
        UE_LOG(LogTemp, Warning, TEXT("Threat added: %s - Danger Level: %.1f"), 
               *ThreatName, EnvironmentalState.DangerLevel);
    }
}

void UEng_SurvivalArchitecture::RemoveThreat(const FString& ThreatName)
{
    if (EnvironmentalState.NearbyThreats.Contains(ThreatName))
    {
        EnvironmentalState.NearbyThreats.Remove(ThreatName);
        EnvironmentalState.DangerLevel = FMath::Min(EnvironmentalState.NearbyThreats.Num() * 20.0f, 100.0f);
        
        // Reduce fear when threat is removed
        ModifyFear(-10.0f);
        
        UE_LOG(LogTemp, Warning, TEXT("Threat removed: %s - Danger Level: %.1f"), 
               *ThreatName, EnvironmentalState.DangerLevel);
    }
}

bool UEng_SurvivalArchitecture::EnforceArchitectureRules()
{
    bool bEnforcementSuccess = true;
    
    // Enforce stat boundaries
    ValidateStatRanges();
    
    // Enforce environmental constraints
    EnvironmentalState.TimeOfDay = FMath::Fmod(EnvironmentalState.TimeOfDay, 24.0f);
    if (EnvironmentalState.TimeOfDay < 0.0f)
    {
        EnvironmentalState.TimeOfDay += 24.0f;
    }
    
    EnvironmentalState.DangerLevel = FMath::Clamp(EnvironmentalState.DangerLevel, 0.0f, 100.0f);
    EnvironmentalState.Humidity = FMath::Clamp(EnvironmentalState.Humidity, 0.0f, 100.0f);
    
    // Enforce architecture rules constraints
    ArchitectureRules.HungerDecayRate = FMath::Max(ArchitectureRules.HungerDecayRate, 0.1f);
    ArchitectureRules.ThirstDecayRate = FMath::Max(ArchitectureRules.ThirstDecayRate, 0.1f);
    ArchitectureRules.StaminaRegenRate = FMath::Max(ArchitectureRules.StaminaRegenRate, 1.0f);
    
    return bEnforcementSuccess;
}

TArray<FString> UEng_SurvivalArchitecture::GetArchitectureViolations() const
{
    return ArchitectureViolations;
}

void UEng_SurvivalArchitecture::LogArchitectureStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== SURVIVAL ARCHITECTURE STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Health: %.1f/%.1f"), CurrentStats.Health, CurrentStats.MaxHealth);
    UE_LOG(LogTemp, Warning, TEXT("Hunger: %.1f/%.1f"), CurrentStats.Hunger, CurrentStats.MaxHunger);
    UE_LOG(LogTemp, Warning, TEXT("Thirst: %.1f/%.1f"), CurrentStats.Thirst, CurrentStats.MaxThirst);
    UE_LOG(LogTemp, Warning, TEXT("Stamina: %.1f/%.1f"), CurrentStats.Stamina, CurrentStats.MaxStamina);
    UE_LOG(LogTemp, Warning, TEXT("Temperature: %.1f°C"), CurrentStats.Temperature);
    UE_LOG(LogTemp, Warning, TEXT("Fear: %.1f/%.1f"), CurrentStats.Fear, CurrentStats.MaxFear);
    UE_LOG(LogTemp, Warning, TEXT("Danger Level: %.1f"), EnvironmentalState.DangerLevel);
    UE_LOG(LogTemp, Warning, TEXT("Active Threats: %d"), EnvironmentalState.NearbyThreats.Num());
    UE_LOG(LogTemp, Warning, TEXT("Architecture Violations: %d"), ArchitectureViolations.Num());
}

void UEng_SurvivalArchitecture::ValidateStatRanges()
{
    CurrentStats.Health = FMath::Clamp(CurrentStats.Health, 0.0f, CurrentStats.MaxHealth);
    CurrentStats.Hunger = FMath::Clamp(CurrentStats.Hunger, 0.0f, CurrentStats.MaxHunger);
    CurrentStats.Thirst = FMath::Clamp(CurrentStats.Thirst, 0.0f, CurrentStats.MaxThirst);
    CurrentStats.Stamina = FMath::Clamp(CurrentStats.Stamina, 0.0f, CurrentStats.MaxStamina);
    CurrentStats.Fear = FMath::Clamp(CurrentStats.Fear, 0.0f, CurrentStats.MaxFear);
    CurrentStats.Temperature = FMath::Clamp(CurrentStats.Temperature, 20.0f, 45.0f);
}

void UEng_SurvivalArchitecture::ApplyEnvironmentalEffects(float DeltaTime)
{
    // Temperature effects on health
    if (CurrentStats.Temperature < 35.0f || CurrentStats.Temperature > 40.0f)
    {
        float HealthLoss = FMath::Abs(CurrentStats.Temperature - 37.0f) * 0.5f * DeltaTime;
        ModifyHealth(-HealthLoss);
    }
    
    // Extreme conditions
    if (CurrentStats.Temperature < 30.0f)
    {
        ModifyStamina(-2.0f * DeltaTime); // Cold drains stamina
    }
    
    if (CurrentStats.Temperature > 42.0f)
    {
        ModifyThirst(-1.0f * DeltaTime); // Heat increases thirst
    }
}

void UEng_SurvivalArchitecture::ProcessSurvivalDecay(float DeltaTime)
{
    // Apply decay rates
    ModifyHunger(-ArchitectureRules.HungerDecayRate * DeltaTime);
    ModifyThirst(-ArchitectureRules.ThirstDecayRate * DeltaTime);
    ModifyFear(-ArchitectureRules.FearDecayRate * DeltaTime);
    
    // Regenerate stamina if not exhausted
    if (CurrentStats.Stamina < CurrentStats.MaxStamina)
    {
        ModifyStamina(ArchitectureRules.StaminaRegenRate * DeltaTime);
    }
    
    // Health effects from low hunger/thirst
    if (CurrentStats.Hunger < 20.0f)
    {
        ModifyHealth(-0.5f * DeltaTime);
    }
    
    if (CurrentStats.Thirst < 15.0f)
    {
        ModifyHealth(-1.0f * DeltaTime);
    }
}

void UEng_SurvivalArchitecture::UpdateThreatAssessment()
{
    if (!ArchitectureRules.bEnableThreatSystem) return;
    
    // Update danger level based on current threats
    float CalculatedDanger = 0.0f;
    
    for (const FString& Threat : EnvironmentalState.NearbyThreats)
    {
        if (Threat.Contains("TRex"))
        {
            CalculatedDanger += 40.0f;
        }
        else if (Threat.Contains("Raptor"))
        {
            CalculatedDanger += 25.0f;
        }
        else if (Threat.Contains("Environment"))
        {
            CalculatedDanger += 10.0f;
        }
        else
        {
            CalculatedDanger += 15.0f;
        }
    }
    
    EnvironmentalState.DangerLevel = FMath::Min(CalculatedDanger, 100.0f);
    
    // Adjust fear based on danger level
    if (EnvironmentalState.DangerLevel > 50.0f)
    {
        ModifyFear(2.0f * GetWorld()->GetDeltaSeconds());
    }
}