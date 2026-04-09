// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "JurassicCharacterMasterSystemV43.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogJurassicCharacterMaster);

UJurassicCharacterMasterSystemV43::UJurassicCharacterMasterSystemV43()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    bIsSystemInitialized = false;
    CurrentEnvironmentalStress = 0.0f;
    CurrentSurvivalMode = ESurvivalMode::Normal;
    CharacterCount = 0;
    MaxCharacterCount = 1000;
    
    // Initialize survival parameters
    SurvivalParameters.HungerRate = 0.1f;
    SurvivalParameters.ThirstRate = 0.15f;
    SurvivalParameters.FatigueRate = 0.05f;
    SurvivalParameters.HealthRegenRate = 0.02f;
    SurvivalParameters.StaminaRegenRate = 0.3f;
    SurvivalParameters.TemperatureAdaptationRate = 0.1f;
    SurvivalParameters.StressDecayRate = 0.05f;
    SurvivalParameters.FearDecayRate = 0.08f;
    
    // Initialize environmental factors
    EnvironmentalFactors.Temperature = 25.0f; // Celsius
    EnvironmentalFactors.Humidity = 0.7f;
    EnvironmentalFactors.WindSpeed = 5.0f;
    EnvironmentalFactors.RainIntensity = 0.0f;
    EnvironmentalFactors.DangerLevel = 0.3f;
    EnvironmentalFactors.ResourceAvailability = 0.8f;
    EnvironmentalFactors.TimeOfDay = 12.0f; // Noon
    EnvironmentalFactors.Season = ESeason::Summer;
    
    // Initialize character archetypes
    InitializeCharacterArchetypes();
}

void UJurassicCharacterMasterSystemV43::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogJurassicCharacterMaster, Log, TEXT("Jurassic Character Master System V43 initializing..."));
    
    // Initialize the system
    InitializeSystem();
    
    // Start environmental monitoring
    StartEnvironmentalMonitoring();
    
    // Register with other systems
    RegisterWithGameSystems();
    
    bIsSystemInitialized = true;
    
    UE_LOG(LogJurassicCharacterMaster, Log, TEXT("Jurassic Character Master System V43 initialized successfully"));
}

void UJurassicCharacterMasterSystemV43::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsSystemInitialized) return;
    
    // Update environmental factors
    UpdateEnvironmentalFactors(DeltaTime);
    
    // Update all active characters
    UpdateActiveCharacters(DeltaTime);
    
    // Process survival mechanics
    ProcessSurvivalMechanics(DeltaTime);
    
    // Update character interactions
    UpdateCharacterInteractions(DeltaTime);
    
    // Monitor system performance
    MonitorSystemPerformance();
}

void UJurassicCharacterMasterSystemV43::InitializeSystem()
{
    // Initialize character pools
    CharacterPools.Empty();
    CharacterPools.Add(ECharacterArchetype::Protagonist, TArray<AJurassicCharacter*>());
    CharacterPools.Add(ECharacterArchetype::Survivor, TArray<AJurassicCharacter*>());
    CharacterPools.Add(ECharacterArchetype::Scientist, TArray<AJurassicCharacter*>());
    CharacterPools.Add(ECharacterArchetype::Hunter, TArray<AJurassicCharacter*>());
    CharacterPools.Add(ECharacterArchetype::Tribal, TArray<AJurassicCharacter*>());
    CharacterPools.Add(ECharacterArchetype::Mystic, TArray<AJurassicCharacter*>());
    
    // Initialize behavior trees
    LoadBehaviorTrees();
    
    // Initialize animation blueprints
    LoadAnimationBlueprints();
    
    // Initialize material libraries
    LoadMaterialLibraries();
    
    UE_LOG(LogJurassicCharacterMaster, Log, TEXT("System initialization complete"));
}

void UJurassicCharacterMasterSystemV43::InitializeCharacterArchetypes()
{
    // Protagonist archetype
    FJurassicCharacterArchetype ProtagonistArchetype;
    ProtagonistArchetype.ArchetypeID = ECharacterArchetype::Protagonist;
    ProtagonistArchetype.Name = TEXT("Paleontologist Protagonist");
    ProtagonistArchetype.Description = TEXT("Time-displaced scientist adapting to prehistoric survival");
    ProtagonistArchetype.BaseStats.Health = 100.0f;
    ProtagonistArchetype.BaseStats.Stamina = 80.0f;
    ProtagonistArchetype.BaseStats.Intelligence = 90.0f;
    ProtagonistArchetype.BaseStats.Strength = 60.0f;
    ProtagonistArchetype.BaseStats.Agility = 70.0f;
    ProtagonistArchetype.BaseStats.Perception = 85.0f;
    ProtagonistArchetype.BaseStats.Charisma = 75.0f;
    ProtagonistArchetype.BaseStats.Survival = 50.0f; // Starts low, grows with experience
    CharacterArchetypes.Add(ECharacterArchetype::Protagonist, ProtagonistArchetype);
    
    // Survivor archetype
    FJurassicCharacterArchetype SurvivorArchetype;
    SurvivorArchetype.ArchetypeID = ECharacterArchetype::Survivor;
    SurvivorArchetype.Name = TEXT("Hardened Survivor");
    SurvivorArchetype.Description = TEXT("Experienced in harsh environments and dangerous situations");
    SurvivorArchetype.BaseStats.Health = 120.0f;
    SurvivorArchetype.BaseStats.Stamina = 100.0f;
    SurvivorArchetype.BaseStats.Intelligence = 70.0f;
    SurvivorArchetype.BaseStats.Strength = 85.0f;
    SurvivorArchetype.BaseStats.Agility = 80.0f;
    SurvivorArchetype.BaseStats.Perception = 90.0f;
    SurvivorArchetype.BaseStats.Charisma = 60.0f;
    SurvivorArchetype.BaseStats.Survival = 95.0f;
    CharacterArchetypes.Add(ECharacterArchetype::Survivor, SurvivorArchetype);
    
    // Scientist archetype
    FJurassicCharacterArchetype ScientistArchetype;
    ScientistArchetype.ArchetypeID = ECharacterArchetype::Scientist;
    ScientistArchetype.Name = TEXT("Research Scientist");
    ScientistArchetype.Description = TEXT("Brilliant mind focused on understanding the prehistoric world");
    ScientistArchetype.BaseStats.Health = 80.0f;
    ScientistArchetype.BaseStats.Stamina = 60.0f;
    ScientistArchetype.BaseStats.Intelligence = 95.0f;
    ScientistArchetype.BaseStats.Strength = 50.0f;
    ScientistArchetype.BaseStats.Agility = 55.0f;
    ScientistArchetype.BaseStats.Perception = 95.0f;
    ScientistArchetype.BaseStats.Charisma = 70.0f;
    ScientistArchetype.BaseStats.Survival = 40.0f;
    CharacterArchetypes.Add(ECharacterArchetype::Scientist, ScientistArchetype);
    
    // Hunter archetype
    FJurassicCharacterArchetype HunterArchetype;
    HunterArchetype.ArchetypeID = ECharacterArchetype::Hunter;
    HunterArchetype.Name = TEXT("Prehistoric Hunter");
    HunterArchetype.Description = TEXT("Skilled tracker and predator, master of stealth and combat");
    HunterArchetype.BaseStats.Health = 110.0f;
    HunterArchetype.BaseStats.Stamina = 90.0f;
    HunterArchetype.BaseStats.Intelligence = 65.0f;
    HunterArchetype.BaseStats.Strength = 90.0f;
    HunterArchetype.BaseStats.Agility = 95.0f;
    HunterArchetype.BaseStats.Perception = 100.0f;
    HunterArchetype.BaseStats.Charisma = 55.0f;
    HunterArchetype.BaseStats.Survival = 90.0f;
    CharacterArchetypes.Add(ECharacterArchetype::Hunter, HunterArchetype);
    
    // Tribal archetype
    FJurassicCharacterArchetype TribalArchetype;
    TribalArchetype.ArchetypeID = ECharacterArchetype::Tribal;
    TribalArchetype.Name = TEXT("Tribal Member");
    TribalArchetype.Description = TEXT("Community-focused individual with deep environmental knowledge");
    TribalArchetype.BaseStats.Health = 95.0f;
    TribalArchetype.BaseStats.Stamina = 85.0f;
    TribalArchetype.BaseStats.Intelligence = 75.0f;
    TribalArchetype.BaseStats.Strength = 75.0f;
    TribalArchetype.BaseStats.Agility = 75.0f;
    TribalArchetype.BaseStats.Perception = 85.0f;
    TribalArchetype.BaseStats.Charisma = 85.0f;
    TribalArchetype.BaseStats.Survival = 85.0f;
    CharacterArchetypes.Add(ECharacterArchetype::Tribal, TribalArchetype);
    
    // Mystic archetype
    FJurassicCharacterArchetype MysticArchetype;
    MysticArchetype.ArchetypeID = ECharacterArchetype::Mystic;
    MysticArchetype.Name = TEXT("Spiritual Mystic");
    MysticArchetype.Description = TEXT("Wise individual with deep understanding of natural and spiritual forces");
    MysticArchetype.BaseStats.Health = 85.0f;
    MysticArchetype.BaseStats.Stamina = 70.0f;
    MysticArchetype.BaseStats.Intelligence = 90.0f;
    MysticArchetype.BaseStats.Strength = 55.0f;
    MysticArchetype.BaseStats.Agility = 65.0f;
    MysticArchetype.BaseStats.Perception = 100.0f;
    MysticArchetype.BaseStats.Charisma = 95.0f;
    MysticArchetype.BaseStats.Survival = 80.0f;
    CharacterArchetypes.Add(ECharacterArchetype::Mystic, MysticArchetype);
}

AJurassicCharacter* UJurassicCharacterMasterSystemV43::CreateCharacter(ECharacterArchetype ArchetypeID, const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
    if (!CharacterArchetypes.Contains(ArchetypeID))
    {
        UE_LOG(LogJurassicCharacterMaster, Error, TEXT("Unknown character archetype: %d"), (int32)ArchetypeID);
        return nullptr;
    }
    
    if (CharacterCount >= MaxCharacterCount)
    {
        UE_LOG(LogJurassicCharacterMaster, Warning, TEXT("Maximum character count reached: %d"), MaxCharacterCount);
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogJurassicCharacterMaster, Error, TEXT("Cannot create character: World is null"));
        return nullptr;
    }
    
    // Get archetype data
    const FJurassicCharacterArchetype& Archetype = CharacterArchetypes[ArchetypeID];
    
    // Spawn character
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AJurassicCharacter* NewCharacter = World->SpawnActor<AJurassicCharacter>(
        AJurassicCharacter::StaticClass(),
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );
    
    if (NewCharacter)
    {
        // Initialize character with archetype data
        InitializeCharacterWithArchetype(NewCharacter, Archetype);
        
        // Add to character pool
        if (!CharacterPools.Contains(ArchetypeID))
        {
            CharacterPools.Add(ArchetypeID, TArray<AJurassicCharacter*>());
        }
        CharacterPools[ArchetypeID].Add(NewCharacter);
        
        // Register character
        ActiveCharacters.Add(NewCharacter);
        CharacterCount++;
        
        UE_LOG(LogJurassicCharacterMaster, Log, TEXT("Created character of type %s at location %s"), 
               *Archetype.Name, *SpawnLocation.ToString());
    }
    
    return NewCharacter;
}

void UJurassicCharacterMasterSystemV43::InitializeCharacterWithArchetype(AJurassicCharacter* Character, const FJurassicCharacterArchetype& Archetype)
{
    if (!Character) return;
    
    // Set base stats
    Character->SetMaxHealth(Archetype.BaseStats.Health);
    Character->SetHealth(Archetype.BaseStats.Health);
    Character->SetMaxStamina(Archetype.BaseStats.Stamina);
    Character->SetStamina(Archetype.BaseStats.Stamina);
    
    // Set character attributes
    Character->SetIntelligence(Archetype.BaseStats.Intelligence);
    Character->SetStrength(Archetype.BaseStats.Strength);
    Character->SetAgility(Archetype.BaseStats.Agility);
    Character->SetPerception(Archetype.BaseStats.Perception);
    Character->SetCharisma(Archetype.BaseStats.Charisma);
    Character->SetSurvivalSkill(Archetype.BaseStats.Survival);
    
    // Set archetype-specific properties
    Character->SetArchetype(Archetype.ArchetypeID);
    Character->SetCharacterName(Archetype.Name);
    
    // Apply diversity profile
    if (DiversityEngine)
    {
        FCharacterDiversityProfile DiversityProfile = DiversityEngine->GenerateRandomDiversityProfile();
        DiversityEngine->ApplyDiversityProfile(Character->GetMesh(), DiversityProfile);
    }
    
    // Set behavior tree based on archetype
    SetCharacterBehaviorTree(Character, Archetype.ArchetypeID);
    
    // Initialize survival state
    InitializeCharacterSurvivalState(Character);
}

void UJurassicCharacterMasterSystemV43::UpdateEnvironmentalFactors(float DeltaTime)
{
    // Update time of day
    EnvironmentalFactors.TimeOfDay += DeltaTime / 3600.0f; // Convert seconds to hours
    if (EnvironmentalFactors.TimeOfDay >= 24.0f)
    {
        EnvironmentalFactors.TimeOfDay -= 24.0f;
    }
    
    // Update temperature based on time of day
    float TimeOfDayFactor = FMath::Sin((EnvironmentalFactors.TimeOfDay - 6.0f) * PI / 12.0f);
    float BaseTemperature = GetSeasonalBaseTemperature();
    EnvironmentalFactors.Temperature = BaseTemperature + (TimeOfDayFactor * 10.0f);
    
    // Update environmental stress
    UpdateEnvironmentalStress(DeltaTime);
    
    // Update weather patterns
    UpdateWeatherPatterns(DeltaTime);
    
    // Update danger level based on time and environment
    UpdateDangerLevel(DeltaTime);
}

void UJurassicCharacterMasterSystemV43::UpdateActiveCharacters(float DeltaTime)
{
    for (int32 i = ActiveCharacters.Num() - 1; i >= 0; --i)
    {
        AJurassicCharacter* Character = ActiveCharacters[i];
        
        if (!IsValid(Character))
        {
            // Remove invalid characters
            ActiveCharacters.RemoveAt(i);
            CharacterCount--;
            continue;
        }
        
        // Update character survival state
        UpdateCharacterSurvivalState(Character, DeltaTime);
        
        // Update character behavior based on environment
        UpdateCharacterBehavior(Character, DeltaTime);
        
        // Update character stress and fear
        UpdateCharacterPsychologicalState(Character, DeltaTime);
    }
}

void UJurassicCharacterMasterSystemV43::ProcessSurvivalMechanics(float DeltaTime)
{
    for (AJurassicCharacter* Character : ActiveCharacters)
    {
        if (!IsValid(Character)) continue;
        
        // Process hunger
        float HungerDecrease = SurvivalParameters.HungerRate * DeltaTime;
        Character->ModifyHunger(-HungerDecrease);
        
        // Process thirst
        float ThirstDecrease = SurvivalParameters.ThirstRate * DeltaTime;
        Character->ModifyThirst(-ThirstDecrease);
        
        // Process fatigue
        float FatigueIncrease = SurvivalParameters.FatigueRate * DeltaTime;
        Character->ModifyFatigue(FatigueIncrease);
        
        // Process health regeneration
        if (Character->GetHealth() < Character->GetMaxHealth())
        {
            float HealthRegen = SurvivalParameters.HealthRegenRate * DeltaTime;
            Character->ModifyHealth(HealthRegen);
        }
        
        // Process stamina regeneration
        if (Character->GetStamina() < Character->GetMaxStamina())
        {
            float StaminaRegen = SurvivalParameters.StaminaRegenRate * DeltaTime;
            Character->ModifyStamina(StaminaRegen);
        }
        
        // Check survival thresholds
        CheckSurvivalThresholds(Character);
    }
}

void UJurassicCharacterMasterSystemV43::UpdateCharacterInteractions(float DeltaTime)
{
    // Process character-to-character interactions
    for (int32 i = 0; i < ActiveCharacters.Num(); ++i)
    {
        for (int32 j = i + 1; j < ActiveCharacters.Num(); ++j)
        {
            AJurassicCharacter* CharacterA = ActiveCharacters[i];
            AJurassicCharacter* CharacterB = ActiveCharacters[j];
            
            if (!IsValid(CharacterA) || !IsValid(CharacterB)) continue;
            
            float Distance = FVector::Dist(CharacterA->GetActorLocation(), CharacterB->GetActorLocation());
            
            // Process interactions within interaction range
            if (Distance <= InteractionRange)
            {
                ProcessCharacterInteraction(CharacterA, CharacterB, DeltaTime);
            }
        }
    }
}

void UJurassicCharacterMasterSystemV43::StartEnvironmentalMonitoring()
{
    UWorld* World = GetWorld();
    if (World)
    {
        // Start timer for environmental updates
        World->GetTimerManager().SetTimer(
            EnvironmentalUpdateTimer,
            this,
            &UJurassicCharacterMasterSystemV43::UpdateEnvironmentalFactors,
            1.0f, // Update every second
            true
        );
    }
}

void UJurassicCharacterMasterSystemV43::RegisterWithGameSystems()
{
    // Register with other game systems for communication
    UE_LOG(LogJurassicCharacterMaster, Log, TEXT("Registering with game systems..."));
    
    // This would typically register with:
    // - World generation system
    // - AI behavior system
    // - Audio system
    // - Lighting system
    // - Quest system
    // etc.
}

float UJurassicCharacterMasterSystemV43::GetSeasonalBaseTemperature() const
{
    switch (EnvironmentalFactors.Season)
    {
        case ESeason::Spring:
            return 20.0f;
        case ESeason::Summer:
            return 30.0f;
        case ESeason::Autumn:
            return 18.0f;
        case ESeason::Winter:
            return 10.0f;
        default:
            return 25.0f;
    }
}

void UJurassicCharacterMasterSystemV43::UpdateEnvironmentalStress(float DeltaTime)
{
    float TargetStress = 0.0f;
    
    // Calculate stress from temperature
    float TemperatureStress = FMath::Abs(EnvironmentalFactors.Temperature - 25.0f) / 25.0f;
    TargetStress += TemperatureStress * 0.3f;
    
    // Calculate stress from weather
    TargetStress += EnvironmentalFactors.RainIntensity * 0.2f;
    TargetStress += (EnvironmentalFactors.WindSpeed / 20.0f) * 0.1f;
    
    // Calculate stress from danger level
    TargetStress += EnvironmentalFactors.DangerLevel * 0.4f;
    
    // Smooth transition to target stress
    CurrentEnvironmentalStress = FMath::FInterpTo(CurrentEnvironmentalStress, TargetStress, DeltaTime, 2.0f);
}

void UJurassicCharacterMasterSystemV43::UpdateWeatherPatterns(float DeltaTime)
{
    // Simple weather simulation
    static float WeatherTimer = 0.0f;
    WeatherTimer += DeltaTime;
    
    // Update rain intensity with some randomness
    if (WeatherTimer > 60.0f) // Check every minute
    {
        float RandomFactor = FMath::RandRange(-0.1f, 0.1f);
        EnvironmentalFactors.RainIntensity = FMath::Clamp(EnvironmentalFactors.RainIntensity + RandomFactor, 0.0f, 1.0f);
        
        // Update wind speed
        float WindChange = FMath::RandRange(-2.0f, 2.0f);
        EnvironmentalFactors.WindSpeed = FMath::Clamp(EnvironmentalFactors.WindSpeed + WindChange, 0.0f, 30.0f);
        
        WeatherTimer = 0.0f;
    }
}

void UJurassicCharacterMasterSystemV43::UpdateDangerLevel(float DeltaTime)
{
    // Danger level varies with time of day (higher at night)
    float TimeOfDayDanger = 0.2f;
    if (EnvironmentalFactors.TimeOfDay < 6.0f || EnvironmentalFactors.TimeOfDay > 18.0f)
    {
        TimeOfDayDanger = 0.7f; // Higher danger at night
    }
    
    // Add weather-based danger
    float WeatherDanger = EnvironmentalFactors.RainIntensity * 0.3f;
    
    // Combine factors
    float TargetDanger = FMath::Clamp(TimeOfDayDanger + WeatherDanger, 0.0f, 1.0f);
    
    // Smooth transition
    EnvironmentalFactors.DangerLevel = FMath::FInterpTo(EnvironmentalFactors.DangerLevel, TargetDanger, DeltaTime, 1.0f);
}

void UJurassicCharacterMasterSystemV43::MonitorSystemPerformance()
{
    // Monitor performance metrics
    static float PerformanceCheckTimer = 0.0f;
    PerformanceCheckTimer += GetWorld()->GetDeltaSeconds();
    
    if (PerformanceCheckTimer > 5.0f) // Check every 5 seconds
    {
        UE_LOG(LogJurassicCharacterMaster, Log, TEXT("Active Characters: %d/%d"), CharacterCount, MaxCharacterCount);
        UE_LOG(LogJurassicCharacterMaster, Log, TEXT("Environmental Stress: %.2f"), CurrentEnvironmentalStress);
        UE_LOG(LogJurassicCharacterMaster, Log, TEXT("Temperature: %.1f°C, Danger Level: %.2f"), 
               EnvironmentalFactors.Temperature, EnvironmentalFactors.DangerLevel);
        
        PerformanceCheckTimer = 0.0f;
    }
}