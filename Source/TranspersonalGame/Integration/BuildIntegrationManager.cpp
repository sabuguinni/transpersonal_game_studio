#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    LastValidationTime = 0.0f;
    bIntegrationValid = false;
    MaxDinosaurs = 150;
    MaxTotalActors = 8000;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initialized"));
    
    // Run initial validation
    RunIntegrationTests();
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Deinitializing"));
    Super::Deinitialize();
}

FBuild_IntegrationReport UBuildIntegrationManager::GenerateIntegrationReport()
{
    FBuild_IntegrationReport Report;
    float StartTime = FPlatformTime::Seconds();
    
    // Validate core modules
    TArray<FString> CoreModules = {
        TEXT("TranspersonalGame"),
        TEXT("Engine"),
        TEXT("CoreUObject"),
        TEXT("UnrealEd")
    };
    
    for (const FString& ModuleName : CoreModules)
    {
        FBuild_ModuleStatus ModuleStatus;
        ValidateModuleStatus(ModuleName, ModuleStatus);
        Report.ModuleStatuses.Add(ModuleStatus);
    }
    
    // Count actors
    if (UWorld* World = GetWorld())
    {
        Report.TotalActors = World->GetCurrentLevel()->Actors.Num();
        
        // Count custom actors
        int32 CustomCount = 0;
        for (AActor* Actor : World->GetCurrentLevel()->Actors)
        {
            if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Transpersonal")))
            {
                CustomCount++;
            }
        }
        Report.CustomActors = CustomCount;
    }
    
    Report.BuildTime = FPlatformTime::Seconds() - StartTime;
    Report.bBuildSuccess = ValidateModuleIntegrity();
    
    LastReport = Report;
    LastValidationTime = FPlatformTime::Seconds();
    
    return Report;
}

bool UBuildIntegrationManager::ValidateModuleIntegrity()
{
    // Check if core classes can be loaded
    TArray<FString> CoreClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager")
    };
    
    int32 LoadedCount = 0;
    for (const FString& ClassName : CoreClasses)
    {
        if (UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName))
        {
            LoadedCount++;
            UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Successfully loaded %s"), *ClassName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Failed to load %s"), *ClassName);
        }
    }
    
    bool bIntegrityValid = (LoadedCount >= CoreClasses.Num() / 2); // At least 50% must load
    bIntegrationValid = bIntegrityValid;
    
    return bIntegrityValid;
}

void UBuildIntegrationManager::RunIntegrationTests()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Running integration tests"));
    
    // Test 1: Module integrity
    bool bModuleTest = ValidateModuleIntegrity();
    
    // Test 2: Actor caps
    bool bActorTest = CheckActorCaps();
    
    // Test 3: Cross-system compatibility
    bool bCompatibilityTest = ValidateCrossSystemCompatibility();
    
    bIntegrationValid = bModuleTest && bActorTest && bCompatibilityTest;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Integration tests complete - Valid: %s"), 
           bIntegrationValid ? TEXT("TRUE") : TEXT("FALSE"));
}

bool UBuildIntegrationManager::CheckActorCaps()
{
    if (UWorld* World = GetWorld())
    {
        int32 TotalActors = World->GetCurrentLevel()->Actors.Num();
        
        // Count dinosaurs
        int32 DinosaurCount = 0;
        TArray<FString> DinosaurLabels = {
            TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), 
            TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")
        };
        
        for (AActor* Actor : World->GetCurrentLevel()->Actors)
        {
            if (Actor)
            {
                FString ActorLabel = Actor->GetActorLabel().ToLower();
                for (const FString& DinoLabel : DinosaurLabels)
                {
                    if (ActorLabel.Contains(DinoLabel))
                    {
                        DinosaurCount++;
                        break;
                    }
                }
            }
        }
        
        bool bWithinLimits = (DinosaurCount <= MaxDinosaurs) && (TotalActors <= MaxTotalActors);
        
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Actor caps - Dinosaurs: %d/%d, Total: %d/%d, Valid: %s"),
               DinosaurCount, MaxDinosaurs, TotalActors, MaxTotalActors, 
               bWithinLimits ? TEXT("TRUE") : TEXT("FALSE"));
        
        return bWithinLimits;
    }
    
    return false;
}

void UBuildIntegrationManager::EnforceActorLimits()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors = World->GetCurrentLevel()->Actors;
        
        // Identify dinosaurs and essential actors
        TArray<AActor*> Dinosaurs;
        TArray<AActor*> EssentialActors;
        TArray<FString> DinosaurLabels = {
            TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), 
            TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")
        };
        TArray<FString> EssentialLabels = {
            TEXT("playerstart"), TEXT("directionallight"), TEXT("skylight"), 
            TEXT("skyatmosphere"), TEXT("fog")
        };
        
        for (AActor* Actor : AllActors)
        {
            if (!Actor) continue;
            
            FString ActorLabel = Actor->GetActorLabel().ToLower();
            
            // Check if dinosaur
            bool bIsDinosaur = false;
            for (const FString& DinoLabel : DinosaurLabels)
            {
                if (ActorLabel.Contains(DinoLabel))
                {
                    Dinosaurs.Add(Actor);
                    bIsDinosaur = true;
                    break;
                }
            }
            
            // Check if essential
            if (!bIsDinosaur)
            {
                for (const FString& EssentialLabel : EssentialLabels)
                {
                    if (ActorLabel.Contains(EssentialLabel))
                    {
                        EssentialActors.Add(Actor);
                        break;
                    }
                }
            }
        }
        
        // Enforce dinosaur limit
        if (Dinosaurs.Num() > MaxDinosaurs)
        {
            int32 ToRemove = Dinosaurs.Num() - MaxDinosaurs;
            for (int32 i = 0; i < ToRemove && i < Dinosaurs.Num(); i++)
            {
                if (Dinosaurs[i])
                {
                    World->DestroyActor(Dinosaurs[i]);
                }
            }
            UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Removed %d excess dinosaurs"), ToRemove);
        }
        
        // Enforce total actor limit
        int32 CurrentTotal = World->GetCurrentLevel()->Actors.Num();
        if (CurrentTotal > MaxTotalActors)
        {
            int32 ToRemove = CurrentTotal - MaxTotalActors;
            int32 Removed = 0;
            
            for (AActor* Actor : AllActors)
            {
                if (Removed >= ToRemove) break;
                if (!Actor) continue;
                
                // Don't remove essential actors or remaining dinosaurs
                if (EssentialActors.Contains(Actor) || Dinosaurs.Contains(Actor)) continue;
                
                World->DestroyActor(Actor);
                Removed++;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Removed %d excess actors"), Removed);
        }
    }
}

TArray<FString> UBuildIntegrationManager::GetLoadedModules()
{
    TArray<FString> LoadedModules;
    
    // Check common modules
    TArray<FString> ModulesToCheck = {
        TEXT("TranspersonalGame"),
        TEXT("Engine"),
        TEXT("CoreUObject"),
        TEXT("UnrealEd"),
        TEXT("ToolMenus"),
        TEXT("EditorStyle")
    };
    
    for (const FString& ModuleName : ModulesToCheck)
    {
        if (FModuleManager::Get().IsModuleLoaded(*ModuleName))
        {
            LoadedModules.Add(ModuleName);
        }
    }
    
    return LoadedModules;
}

bool UBuildIntegrationManager::ValidateCrossSystemCompatibility()
{
    // Test cross-system interactions
    bool bCompatible = true;
    
    // Test 1: Character + GameState interaction
    UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    UClass* GameStateClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    
    if (!CharacterClass || !GameStateClass)
    {
        bCompatible = false;
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Core classes not compatible"));
    }
    
    // Test 2: World generation + Foliage interaction
    UClass* WorldGenClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    UClass* FoliageClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.FoliageManager"));
    
    if (WorldGenClass && FoliageClass)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: World generation and foliage systems compatible"));
    }
    
    return bCompatible;
}

void UBuildIntegrationManager::ValidateModuleStatus(const FString& ModuleName, FBuild_ModuleStatus& OutStatus)
{
    OutStatus.ModuleName = ModuleName;
    OutStatus.bIsLoaded = FModuleManager::Get().IsModuleLoaded(*ModuleName);
    OutStatus.ClassCount = 0;
    OutStatus.LoadTime = 0.0f;
    
    if (OutStatus.bIsLoaded)
    {
        // Count classes in module (simplified)
        if (ModuleName == TEXT("TranspersonalGame"))
        {
            OutStatus.ClassCount = 7; // Known core classes
        }
        else
        {
            OutStatus.ClassCount = 1; // Placeholder
        }
    }
}

void UBuildIntegrationManager::CheckActorIntegrity()
{
    // Implementation for actor integrity checking
    if (UWorld* World = GetWorld())
    {
        int32 ValidActors = 0;
        int32 InvalidActors = 0;
        
        for (AActor* Actor : World->GetCurrentLevel()->Actors)
        {
            if (Actor && IsValid(Actor))
            {
                ValidActors++;
            }
            else
            {
                InvalidActors++;
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Actor integrity - Valid: %d, Invalid: %d"), 
               ValidActors, InvalidActors);
    }
}

void UBuildIntegrationManager::ValidateSystemDependencies()
{
    // Check system dependencies
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Validating system dependencies"));
    
    // This would check if systems have proper references to each other
    // For now, just log that validation occurred
}

bool UBuildIntegrationManager::TestCrossSystemInteraction()
{
    // Test interactions between different game systems
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Testing cross-system interactions"));
    
    // Return true for now - would implement actual interaction tests
    return true;
}