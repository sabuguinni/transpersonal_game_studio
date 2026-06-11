#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Misc/PackageName.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = true;
    ValidationInterval = 30.0f; // Validate every 30 seconds
    bAutoValidateOnTick = false;
    TimeSinceLastValidation = 0.0f;

    // Initialize required modules
    RequiredModules = {
        TEXT("Core"),
        TEXT("Character"), 
        TEXT("World"),
        TEXT("AI"),
        TEXT("Combat"),
        TEXT("Quest"),
        TEXT("Audio"),
        TEXT("VFX"),
        TEXT("QA"),
        TEXT("Integration")
    };

    // Initialize required classes
    RequiredClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager")
    };
}

void UBuild_IntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Build Integration Validator initialized"));
    
    // Perform initial validation
    if (bAutoValidateOnTick)
    {
        LastReport = ValidateFullIntegration();
        LogIntegrationReport(LastReport);
    }
}

void UBuild_IntegrationValidator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bAutoValidateOnTick)
    {
        TimeSinceLastValidation += DeltaTime;
        
        if (TimeSinceLastValidation >= ValidationInterval)
        {
            LastReport = ValidateFullIntegration();
            TimeSinceLastValidation = 0.0f;
            
            // Log critical issues immediately
            if (LastReport.OverallStatus == EBuild_IntegrationStatus::Critical || 
                LastReport.OverallStatus == EBuild_IntegrationStatus::Failed)
            {
                UE_LOG(LogTemp, Error, TEXT("Integration validation failed! Health: %.1f%%"), LastReport.OverallHealthPercentage);
            }
        }
    }
}

FBuild_IntegrationReport UBuild_IntegrationValidator::ValidateFullIntegration()
{
    FBuild_IntegrationReport Report;
    Report.LastValidationTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Log, TEXT("Starting full integration validation..."));
    
    // Validate each required module
    for (const FString& ModuleName : RequiredModules)
    {
        FBuild_ModuleStatus ModuleStatus = ValidateModule(ModuleName);
        Report.ModuleStatuses.Add(ModuleStatus);
    }
    
    // Count total actors in level
    Report.TotalActorsInLevel = CountActorsInLevel();
    
    // Count loaded classes
    Report.LoadedClassCount = 0;
    for (const FString& ClassName : RequiredClasses)
    {
        if (ValidateClassLoading(ClassName))
        {
            Report.LoadedClassCount++;
        }
    }
    
    // Calculate overall health
    Report.OverallHealthPercentage = CalculateIntegrationHealth();
    Report.OverallStatus = DetermineOverallStatus(Report.OverallHealthPercentage);
    
    UE_LOG(LogTemp, Log, TEXT("Integration validation complete. Health: %.1f%%"), Report.OverallHealthPercentage);
    
    return Report;
}

FBuild_ModuleStatus UBuild_IntegrationValidator::ValidateModule(const FString& ModuleName)
{
    FBuild_ModuleStatus Status;
    Status.ModuleName = ModuleName;
    
    // Check if module is loaded
    if (IsModuleLoaded(ModuleName))
    {
        Status.Status = EBuild_IntegrationStatus::Healthy;
        Status.ClassCount = CountClassesInModule(ModuleName);
        
        // Count actors of this module type in level
        Status.ActorCount = 0;
        if (UWorld* World = GetWorld())
        {
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                AActor* Actor = *ActorItr;
                if (Actor && Actor->GetClass()->GetName().Contains(ModuleName))
                {
                    Status.ActorCount++;
                }
            }
        }
        
        // Check for warnings
        if (Status.ClassCount == 0)
        {
            Status.Status = EBuild_IntegrationStatus::Warning;
            Status.ErrorMessages.Add(FString::Printf(TEXT("Module %s has no classes"), *ModuleName));
        }
    }
    else
    {
        Status.Status = EBuild_IntegrationStatus::Failed;
        Status.ErrorMessages.Add(FString::Printf(TEXT("Module %s is not loaded"), *ModuleName));
    }
    
    return Status;
}

bool UBuild_IntegrationValidator::ValidateClassLoading(const FString& ClassName)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
    return LoadedClass != nullptr;
}

int32 UBuild_IntegrationValidator::CountActorsInLevel()
{
    int32 ActorCount = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            ActorCount++;
        }
    }
    
    return ActorCount;
}

TArray<FString> UBuild_IntegrationValidator::GetLoadedModuleNames()
{
    TArray<FString> LoadedModules;
    
    // This is a simplified implementation
    // In a real scenario, you'd query the module manager
    for (const FString& ModuleName : RequiredModules)
    {
        if (IsModuleLoaded(ModuleName))
        {
            LoadedModules.Add(ModuleName);
        }
    }
    
    return LoadedModules;
}

float UBuild_IntegrationValidator::CalculateIntegrationHealth()
{
    float TotalScore = 0.0f;
    float MaxScore = 0.0f;
    
    // Module health (40% of total score)
    float ModuleScore = 0.0f;
    for (const FBuild_ModuleStatus& ModuleStatus : LastReport.ModuleStatuses)
    {
        switch (ModuleStatus.Status)
        {
            case EBuild_IntegrationStatus::Healthy:
                ModuleScore += 1.0f;
                break;
            case EBuild_IntegrationStatus::Warning:
                ModuleScore += 0.7f;
                break;
            case EBuild_IntegrationStatus::Critical:
                ModuleScore += 0.3f;
                break;
            default:
                ModuleScore += 0.0f;
                break;
        }
    }
    TotalScore += (ModuleScore / RequiredModules.Num()) * 40.0f;
    MaxScore += 40.0f;
    
    // Class loading health (30% of total score)
    float ClassScore = (float)LastReport.LoadedClassCount / RequiredClasses.Num();
    TotalScore += ClassScore * 30.0f;
    MaxScore += 30.0f;
    
    // Level content health (30% of total score)
    float LevelScore = FMath::Min(LastReport.TotalActorsInLevel / 20.0f, 1.0f); // Expect at least 20 actors
    TotalScore += LevelScore * 30.0f;
    MaxScore += 30.0f;
    
    return (TotalScore / MaxScore) * 100.0f;
}

EBuild_IntegrationStatus UBuild_IntegrationValidator::DetermineOverallStatus(float HealthPercentage)
{
    if (HealthPercentage >= 90.0f)
    {
        return EBuild_IntegrationStatus::Healthy;
    }
    else if (HealthPercentage >= 70.0f)
    {
        return EBuild_IntegrationStatus::Warning;
    }
    else if (HealthPercentage >= 40.0f)
    {
        return EBuild_IntegrationStatus::Critical;
    }
    else
    {
        return EBuild_IntegrationStatus::Failed;
    }
}

void UBuild_IntegrationValidator::LogIntegrationReport(const FBuild_IntegrationReport& Report)
{
    UE_LOG(LogTemp, Log, TEXT("=== INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Overall Health: %.1f%%"), Report.OverallHealthPercentage);
    UE_LOG(LogTemp, Log, TEXT("Overall Status: %s"), 
        Report.OverallStatus == EBuild_IntegrationStatus::Healthy ? TEXT("Healthy") :
        Report.OverallStatus == EBuild_IntegrationStatus::Warning ? TEXT("Warning") :
        Report.OverallStatus == EBuild_IntegrationStatus::Critical ? TEXT("Critical") : TEXT("Failed"));
    UE_LOG(LogTemp, Log, TEXT("Total Actors: %d"), Report.TotalActorsInLevel);
    UE_LOG(LogTemp, Log, TEXT("Loaded Classes: %d/%d"), Report.LoadedClassCount, RequiredClasses.Num());
    
    for (const FBuild_ModuleStatus& ModuleStatus : Report.ModuleStatuses)
    {
        UE_LOG(LogTemp, Log, TEXT("Module %s: %s (Classes: %d, Actors: %d)"), 
            *ModuleStatus.ModuleName,
            ModuleStatus.Status == EBuild_IntegrationStatus::Healthy ? TEXT("OK") : TEXT("FAIL"),
            ModuleStatus.ClassCount,
            ModuleStatus.ActorCount);
            
        for (const FString& Error : ModuleStatus.ErrorMessages)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Error);
        }
    }
    UE_LOG(LogTemp, Log, TEXT("=== END REPORT ==="));
}

void UBuild_IntegrationValidator::CreateIntegrationTestActors()
{
    if (UWorld* World = GetWorld())
    {
        // Create test marker
        FVector TestLocation(2000.0f, 2000.0f, 200.0f);
        FRotator TestRotation(0.0f, 0.0f, 0.0f);
        
        AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), TestLocation, TestRotation);
        if (TestActor)
        {
            TestActor->SetActorLabel(TEXT("Integration_Test_Marker"));
            UE_LOG(LogTemp, Log, TEXT("Created integration test marker"));
        }
    }
}

bool UBuild_IntegrationValidator::ValidateMinPlayableMap()
{
    if (UWorld* World = GetWorld())
    {
        int32 ActorCount = CountActorsInLevel();
        
        // Check for minimum required actors
        bool bHasPlayerStart = false;
        bool bHasLighting = false;
        bool bHasTerrain = false;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                FString ActorName = Actor->GetClass()->GetName();
                if (ActorName.Contains(TEXT("PlayerStart")))
                {
                    bHasPlayerStart = true;
                }
                else if (ActorName.Contains(TEXT("Light")))
                {
                    bHasLighting = true;
                }
                else if (ActorName.Contains(TEXT("Landscape")) || ActorName.Contains(TEXT("Terrain")))
                {
                    bHasTerrain = true;
                }
            }
        }
        
        bool bIsValid = (ActorCount >= 10) && bHasPlayerStart && bHasLighting;
        
        UE_LOG(LogTemp, Log, TEXT("MinPlayableMap validation: %s (Actors: %d, PlayerStart: %s, Lighting: %s, Terrain: %s)"),
            bIsValid ? TEXT("PASS") : TEXT("FAIL"),
            ActorCount,
            bHasPlayerStart ? TEXT("YES") : TEXT("NO"),
            bHasLighting ? TEXT("YES") : TEXT("NO"),
            bHasTerrain ? TEXT("YES") : TEXT("NO"));
            
        return bIsValid;
    }
    
    return false;
}

bool UBuild_IntegrationValidator::IsModuleLoaded(const FString& ModuleName)
{
    // Simplified check - in reality you'd use FModuleManager
    // For now, assume core modules are always loaded
    return RequiredModules.Contains(ModuleName);
}

int32 UBuild_IntegrationValidator::CountClassesInModule(const FString& ModuleName)
{
    // Simplified implementation
    // In reality, you'd enumerate classes in the module
    return 1; // Assume each module has at least one class
}

TArray<FString> UBuild_IntegrationValidator::GetModuleErrors(const FString& ModuleName)
{
    TArray<FString> Errors;
    
    if (!IsModuleLoaded(ModuleName))
    {
        Errors.Add(FString::Printf(TEXT("Module %s is not loaded"), *ModuleName));
    }
    
    return Errors;
}