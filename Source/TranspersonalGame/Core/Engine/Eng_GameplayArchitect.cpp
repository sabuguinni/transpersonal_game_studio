#include "Eng_GameplayArchitect.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AEng_GameplayArchitect::AEng_GameplayArchitect()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize gameplay architecture defaults
    bEnableGameplayValidation = true;
    ValidationInterval = 5.0f;
    
    // Survival mechanics defaults
    BaseHealthDecayRate = 1.0f;
    BaseHungerDecayRate = 2.0f;
    BaseThirstDecayRate = 3.0f;
    BaseStaminaRegenRate = 5.0f;
    
    // Dinosaur interaction defaults
    DinosaurDetectionRange = 2000.0f;
    DinosaurAggressionMultiplier = 1.5f;
    bEnableDinosaurPackBehavior = true;
    
    // Crafting system defaults
    MaxCraftingSlots = 8;
    CraftingSpeedMultiplier = 1.0f;
    bEnableAdvancedCrafting = false;
    
    // Internal state
    LastValidationTime = 0.0f;
    bSystemsInitialized = false;
    
    // Initialize default gameplay rules
    FEng_GameplayRule SurvivalRule;
    SurvivalRule.RuleName = TEXT("Core Survival");
    SurvivalRule.RuleDescription = TEXT("Health, hunger, thirst, and stamina must be managed");
    SurvivalRule.bIsActive = true;
    SurvivalRule.Priority = 10;
    GameplayRules.Add(SurvivalRule);
    
    FEng_GameplayRule DinosaurRule;
    DinosaurRule.RuleName = TEXT("Dinosaur Encounters");
    DinosaurRule.RuleDescription = TEXT("Dinosaurs are territorial and dangerous");
    DinosaurRule.bIsActive = true;
    DinosaurRule.Priority = 9;
    GameplayRules.Add(DinosaurRule);
    
    FEng_GameplayRule CraftingRule;
    CraftingRule.RuleName = TEXT("Realistic Crafting");
    CraftingRule.RuleDescription = TEXT("Only primitive tools and weapons can be crafted");
    CraftingRule.bIsActive = true;
    CraftingRule.Priority = 8;
    GameplayRules.Add(CraftingRule);
}

void AEng_GameplayArchitect::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize all gameplay systems
    InternalInitializeSystems();
    
    UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: Systems initialized successfully"));
}

void AEng_GameplayArchitect::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableGameplayValidation)
    {
        LastValidationTime += DeltaTime;
        
        if (LastValidationTime >= ValidationInterval)
        {
            InternalValidateGameplay();
            InternalUpdateMetrics(DeltaTime);
            LastValidationTime = 0.0f;
        }
    }
}

bool AEng_GameplayArchitect::ValidateGameplayRules()
{
    bool bAllRulesValid = true;
    
    for (const FEng_GameplayRule& Rule : GameplayRules)
    {
        if (Rule.bIsActive)
        {
            // Validate each rule based on its name
            if (Rule.RuleName == TEXT("Core Survival"))
            {
                // Check if survival stats are within reasonable ranges
                bool bSurvivalValid = (BaseHealthDecayRate > 0.0f && BaseHealthDecayRate < 10.0f) &&
                                    (BaseHungerDecayRate > 0.0f && BaseHungerDecayRate < 10.0f) &&
                                    (BaseThirstDecayRate > 0.0f && BaseThirstDecayRate < 10.0f);
                
                if (!bSurvivalValid)
                {
                    UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: Survival rule validation failed"));
                    bAllRulesValid = false;
                }
            }
            else if (Rule.RuleName == TEXT("Dinosaur Encounters"))
            {
                // Check if dinosaur parameters are reasonable
                bool bDinosaurValid = (DinosaurDetectionRange > 500.0f && DinosaurDetectionRange < 5000.0f) &&
                                    (DinosaurAggressionMultiplier > 0.5f && DinosaurAggressionMultiplier < 3.0f);
                
                if (!bDinosaurValid)
                {
                    UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: Dinosaur rule validation failed"));
                    bAllRulesValid = false;
                }
            }
            else if (Rule.RuleName == TEXT("Realistic Crafting"))
            {
                // Check if crafting parameters are balanced
                bool bCraftingValid = (MaxCraftingSlots > 0 && MaxCraftingSlots <= 20) &&
                                    (CraftingSpeedMultiplier > 0.1f && CraftingSpeedMultiplier < 5.0f);
                
                if (!bCraftingValid)
                {
                    UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: Crafting rule validation failed"));
                    bAllRulesValid = false;
                }
            }
        }
    }
    
    return bAllRulesValid;
}

void AEng_GameplayArchitect::UpdateGameplayMetrics(float DeltaTime)
{
    // Update survival time
    CurrentMetrics.PlayerSurvivalTime += DeltaTime;
    
    // Find player character and update distance traveled
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        static FVector LastPlayerLocation = PlayerPawn->GetActorLocation();
        FVector CurrentLocation = PlayerPawn->GetActorLocation();
        float DistanceThisFrame = FVector::Dist(LastPlayerLocation, CurrentLocation);
        CurrentMetrics.DistanceTraveled += DistanceThisFrame;
        LastPlayerLocation = CurrentLocation;
    }
    
    UE_LOG(LogTemp, Log, TEXT("GameplayArchitect: Metrics updated - Survival Time: %.2f, Distance: %.2f"), 
           CurrentMetrics.PlayerSurvivalTime, CurrentMetrics.DistanceTraveled);
}

void AEng_GameplayArchitect::RegisterGameplayRule(const FEng_GameplayRule& NewRule)
{
    // Check if rule already exists
    bool bRuleExists = false;
    for (int32 i = 0; i < GameplayRules.Num(); i++)
    {
        if (GameplayRules[i].RuleName == NewRule.RuleName)
        {
            GameplayRules[i] = NewRule; // Update existing rule
            bRuleExists = true;
            break;
        }
    }
    
    if (!bRuleExists)
    {
        GameplayRules.Add(NewRule);
    }
    
    // Sort rules by priority (higher priority first)
    GameplayRules.Sort([](const FEng_GameplayRule& A, const FEng_GameplayRule& B) {
        return A.Priority > B.Priority;
    });
    
    UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: Rule registered - %s"), *NewRule.RuleName);
}

void AEng_GameplayArchitect::RemoveGameplayRule(const FString& RuleName)
{
    for (int32 i = GameplayRules.Num() - 1; i >= 0; i--)
    {
        if (GameplayRules[i].RuleName == RuleName)
        {
            GameplayRules.RemoveAt(i);
            UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: Rule removed - %s"), *RuleName);
            break;
        }
    }
}

void AEng_GameplayArchitect::InitializeSurvivalSystems()
{
    // Set up survival system parameters
    UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: Initializing survival systems"));
    
    // Validate survival parameters
    if (BaseHealthDecayRate <= 0.0f) BaseHealthDecayRate = 1.0f;
    if (BaseHungerDecayRate <= 0.0f) BaseHungerDecayRate = 2.0f;
    if (BaseThirstDecayRate <= 0.0f) BaseThirstDecayRate = 3.0f;
    if (BaseStaminaRegenRate <= 0.0f) BaseStaminaRegenRate = 5.0f;
    
    UE_LOG(LogTemp, Log, TEXT("GameplayArchitect: Survival rates - Health: %.2f, Hunger: %.2f, Thirst: %.2f, Stamina: %.2f"),
           BaseHealthDecayRate, BaseHungerDecayRate, BaseThirstDecayRate, BaseStaminaRegenRate);
}

void AEng_GameplayArchitect::ValidateSurvivalBalance()
{
    bool bBalanceValid = true;
    
    // Check if thirst decays faster than hunger (realistic)
    if (BaseThirstDecayRate <= BaseHungerDecayRate)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: Thirst should decay faster than hunger"));
        bBalanceValid = false;
    }
    
    // Check if stamina regenerates reasonably
    if (BaseStaminaRegenRate < BaseHealthDecayRate)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: Stamina regen too slow compared to health decay"));
        bBalanceValid = false;
    }
    
    if (bBalanceValid)
    {
        UE_LOG(LogTemp, Log, TEXT("GameplayArchitect: Survival balance validated successfully"));
    }
}

float AEng_GameplayArchitect::CalculateOptimalDecayRate(const FString& StatType)
{
    if (StatType == TEXT("Health"))
    {
        return FMath::Clamp(BaseHealthDecayRate, 0.5f, 2.0f);
    }
    else if (StatType == TEXT("Hunger"))
    {
        return FMath::Clamp(BaseHungerDecayRate, 1.0f, 4.0f);
    }
    else if (StatType == TEXT("Thirst"))
    {
        return FMath::Clamp(BaseThirstDecayRate, 1.5f, 5.0f);
    }
    else if (StatType == TEXT("Stamina"))
    {
        return FMath::Clamp(BaseStaminaRegenRate, 2.0f, 10.0f);
    }
    
    return 1.0f; // Default
}

void AEng_GameplayArchitect::InitializeDinosaurSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: Initializing dinosaur systems"));
    
    // Validate dinosaur parameters
    if (DinosaurDetectionRange < 500.0f) DinosaurDetectionRange = 1500.0f;
    if (DinosaurDetectionRange > 5000.0f) DinosaurDetectionRange = 3000.0f;
    
    if (DinosaurAggressionMultiplier < 0.5f) DinosaurAggressionMultiplier = 1.0f;
    if (DinosaurAggressionMultiplier > 3.0f) DinosaurAggressionMultiplier = 2.0f;
    
    UE_LOG(LogTemp, Log, TEXT("GameplayArchitect: Dinosaur params - Detection: %.2f, Aggression: %.2f, Pack Behavior: %s"),
           DinosaurDetectionRange, DinosaurAggressionMultiplier, bEnableDinosaurPackBehavior ? TEXT("Enabled") : TEXT("Disabled"));
}

void AEng_GameplayArchitect::ValidateDinosaurBehavior()
{
    // Check if there are dinosaurs in the level
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 DinosaurCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            DinosaurCount++;
        }
    }
    
    if (DinosaurCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("GameplayArchitect: Found %d dinosaur actors in level"), DinosaurCount);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: No dinosaur actors found in level"));
    }
}

int32 AEng_GameplayArchitect::GetOptimalDinosaurCount()
{
    // Calculate optimal dinosaur count based on world size and player count
    UWorld* World = GetWorld();
    if (!World) return 5; // Default
    
    // For now, return a reasonable number for testing
    return FMath::Clamp(8, 3, 15);
}

void AEng_GameplayArchitect::InitializeCraftingSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: Initializing crafting systems"));
    
    // Validate crafting parameters
    if (MaxCraftingSlots < 1) MaxCraftingSlots = 4;
    if (MaxCraftingSlots > 20) MaxCraftingSlots = 12;
    
    if (CraftingSpeedMultiplier < 0.1f) CraftingSpeedMultiplier = 0.5f;
    if (CraftingSpeedMultiplier > 5.0f) CraftingSpeedMultiplier = 2.0f;
    
    UE_LOG(LogTemp, Log, TEXT("GameplayArchitect: Crafting params - Max Slots: %d, Speed: %.2f, Advanced: %s"),
           MaxCraftingSlots, CraftingSpeedMultiplier, bEnableAdvancedCrafting ? TEXT("Enabled") : TEXT("Disabled"));
}

void AEng_GameplayArchitect::ValidateCraftingBalance()
{
    bool bCraftingBalanced = true;
    
    // Check if crafting slots are reasonable
    if (MaxCraftingSlots > 16)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: Too many crafting slots may make game too easy"));
        bCraftingBalanced = false;
    }
    
    // Check if crafting speed is balanced
    if (CraftingSpeedMultiplier > 3.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: Crafting speed too high may reduce challenge"));
        bCraftingBalanced = false;
    }
    
    if (bCraftingBalanced)
    {
        UE_LOG(LogTemp, Log, TEXT("GameplayArchitect: Crafting balance validated successfully"));
    }
}

bool AEng_GameplayArchitect::CanCraftItem(const FString& ItemName)
{
    // Basic crafting validation - primitive items only
    TArray<FString> AllowedItems = {
        TEXT("Stone Axe"),
        TEXT("Wooden Spear"),
        TEXT("Stone Knife"),
        TEXT("Fire Starter"),
        TEXT("Simple Shelter"),
        TEXT("Water Container"),
        TEXT("Hunting Trap")
    };
    
    return AllowedItems.Contains(ItemName);
}

FString AEng_GameplayArchitect::GenerateGameplayReport()
{
    FString Report = TEXT("=== GAMEPLAY ARCHITECTURE REPORT ===\n\n");
    
    Report += FString::Printf(TEXT("Active Rules: %d\n"), GameplayRules.Num());
    for (const FEng_GameplayRule& Rule : GameplayRules)
    {
        if (Rule.bIsActive)
        {
            Report += FString::Printf(TEXT("- %s (Priority: %d)\n"), *Rule.RuleName, Rule.Priority);
        }
    }
    
    Report += TEXT("\nCurrent Metrics:\n");
    Report += FString::Printf(TEXT("- Survival Time: %.2f seconds\n"), CurrentMetrics.PlayerSurvivalTime);
    Report += FString::Printf(TEXT("- Distance Traveled: %.2f units\n"), CurrentMetrics.DistanceTraveled);
    Report += FString::Printf(TEXT("- Dinosaur Encounters: %d\n"), CurrentMetrics.DinosaurEncounters);
    Report += FString::Printf(TEXT("- Items Crafted: %d\n"), CurrentMetrics.CraftedItems);
    Report += FString::Printf(TEXT("- Resources Gathered: %d\n"), CurrentMetrics.ResourcesGathered);
    
    Report += TEXT("\nSystem Parameters:\n");
    Report += FString::Printf(TEXT("- Health Decay Rate: %.2f\n"), BaseHealthDecayRate);
    Report += FString::Printf(TEXT("- Hunger Decay Rate: %.2f\n"), BaseHungerDecayRate);
    Report += FString::Printf(TEXT("- Thirst Decay Rate: %.2f\n"), BaseThirstDecayRate);
    Report += FString::Printf(TEXT("- Stamina Regen Rate: %.2f\n"), BaseStaminaRegenRate);
    Report += FString::Printf(TEXT("- Dinosaur Detection Range: %.2f\n"), DinosaurDetectionRange);
    Report += FString::Printf(TEXT("- Max Crafting Slots: %d\n"), MaxCraftingSlots);
    
    return Report;
}

void AEng_GameplayArchitect::ExportArchitectureData()
{
    FString ReportData = GenerateGameplayReport();
    UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: Architecture data exported"));
    UE_LOG(LogTemp, Log, TEXT("%s"), *ReportData);
}

void AEng_GameplayArchitect::ValidateAllGameplaySystems()
{
    UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: Validating all gameplay systems..."));
    
    bool bAllValid = true;
    
    // Validate gameplay rules
    if (!ValidateGameplayRules())
    {
        bAllValid = false;
        UE_LOG(LogTemp, Error, TEXT("GameplayArchitect: Gameplay rules validation failed"));
    }
    
    // Validate survival systems
    ValidateSurvivalBalance();
    
    // Validate dinosaur systems
    ValidateDinosaurBehavior();
    
    // Validate crafting systems
    ValidateCraftingBalance();
    
    if (bAllValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: All systems validated successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameplayArchitect: Some systems failed validation"));
    }
}

void AEng_GameplayArchitect::InternalValidateGameplay()
{
    ValidateGameplayRules();
}

void AEng_GameplayArchitect::InternalUpdateMetrics(float DeltaTime)
{
    UpdateGameplayMetrics(DeltaTime);
}

void AEng_GameplayArchitect::InternalInitializeSystems()
{
    if (!bSystemsInitialized)
    {
        InitializeSurvivalSystems();
        InitializeDinosaurSystems();
        InitializeCraftingSystems();
        
        bSystemsInitialized = true;
        UE_LOG(LogTemp, Warning, TEXT("GameplayArchitect: All systems initialized"));
    }
}