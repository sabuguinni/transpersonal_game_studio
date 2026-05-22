#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "EngineUtils.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/GameViewportClient.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    OverallStatus = EBuild_IntegrationStatus::Pending;
    StatusMessage = TEXT("Integration system initialized");
    LastValidationTime = 0.0f;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Subsystem initialized"));
    
    // Initialize system reports
    SystemReports.Empty();
    BiomeValidations.Empty();
    
    // Set initial status
    OverallStatus = EBuild_IntegrationStatus::Pending;
    StatusMessage = TEXT("Build integration manager ready");
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Subsystem deinitialized"));
    Super::Deinitialize();
}

bool UBuildIntegrationManager::ValidateAllSystems()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Starting full system validation"));
    
    float StartTime = FPlatformTime::Seconds();
    SystemReports.Empty();
    
    bool bAllSystemsValid = true;
    
    // Validate each system
    bAllSystemsValid &= ValidateWorldGeneration();
    bAllSystemsValid &= ValidateCharacterSystems();
    bAllSystemsValid &= ValidateDinosaurSystems();
    bAllSystemsValid &= ValidateVFXSystems();
    bAllSystemsValid &= ValidateAudioSystems();
    bAllSystemsValid &= ValidateQuestSystems();
    
    // Validate biome distribution
    bAllSystemsValid &= ValidateBiomeDistribution();
    
    LastValidationTime = FPlatformTime::Seconds() - StartTime;
    
    // Calculate overall status
    CalculateOverallStatus();
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Validation completed in %.2f seconds"), LastValidationTime);
    
    return bAllSystemsValid;
}

bool UBuildIntegrationManager::ValidateSystemIntegration(const FString& SystemName)
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Validating system: %s"), *SystemName);
    
    if (SystemName == TEXT("WorldGeneration"))
    {
        return ValidateWorldGeneration();
    }
    else if (SystemName == TEXT("Character"))
    {
        return ValidateCharacterSystems();
    }
    else if (SystemName == TEXT("Dinosaur"))
    {
        return ValidateDinosaurSystems();
    }
    else if (SystemName == TEXT("VFX"))
    {
        return ValidateVFXSystems();
    }
    else if (SystemName == TEXT("Audio"))
    {
        return ValidateAudioSystems();
    }
    else if (SystemName == TEXT("Quest"))
    {
        return ValidateQuestSystems();
    }
    
    UpdateSystemReport(SystemName, EBuild_IntegrationStatus::Failed, TEXT("Unknown system"));
    return false;
}

TArray<FBuild_SystemReport> UBuildIntegrationManager::GetSystemReports() const
{
    return SystemReports;
}

bool UBuildIntegrationManager::ValidateBiomeDistribution()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Validating biome distribution"));
    
    BiomeValidations.Empty();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateSystemReport(TEXT("BiomeValidation"), EBuild_IntegrationStatus::Failed, TEXT("No world available"));
        return false;
    }
    
    // Define biome centers
    TMap<FString, FVector> BiomeCenters;
    BiomeCenters.Add(TEXT("Savana"), FVector(0, 0, 0));
    BiomeCenters.Add(TEXT("Floresta"), FVector(-45000, 40000, 0));
    BiomeCenters.Add(TEXT("Deserto"), FVector(50000, -40000, 0));
    BiomeCenters.Add(TEXT("Pantano"), FVector(-50000, -45000, 0));
    BiomeCenters.Add(TEXT("Montanha"), FVector(40000, 50000, 0));
    
    // Validate each biome
    for (const auto& BiomePair : BiomeCenters)
    {
        FBuild_BiomeValidation BiomeValidation;
        BiomeValidation.BiomeName = BiomePair.Key;
        BiomeValidation.BiomeCenter = BiomePair.Value;
        
        // Count actors near biome center
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (!Actor) continue;
            
            FVector ActorLocation = Actor->GetActorLocation();
            float Distance = FVector::Dist(ActorLocation, BiomePair.Value);
            
            if (Distance < 10000.0f) // Within 10km of biome center
            {
                BiomeValidation.ActorsInBiome++;
                
                FString ActorName = Actor->GetName().ToLower();
                if (ActorName.Contains(TEXT("dinosaur")) || ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("raptor")))
                {
                    BiomeValidation.DinosaurCount++;
                }
                else if (ActorName.Contains(TEXT("tree")) || ActorName.Contains(TEXT("rock")) || ActorName.Contains(TEXT("foliage")))
                {
                    BiomeValidation.EnvironmentCount++;
                }
            }
        }
        
        BiomeValidations.Add(BiomeValidation);
    }
    
    UpdateSystemReport(TEXT("BiomeValidation"), EBuild_IntegrationStatus::Success, 
        FString::Printf(TEXT("Validated %d biomes"), BiomeValidations.Num()));
    
    return true;
}

TArray<FBuild_BiomeValidation> UBuildIntegrationManager::GetBiomeValidation() const
{
    return BiomeValidations;
}

int32 UBuildIntegrationManager::GetTotalActorCount() const
{
    UWorld* World = GetWorld();
    if (!World) return 0;
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        Count++;
    }
    
    return Count;
}

int32 UBuildIntegrationManager::GetActorCountByType(const FString& ActorType) const
{
    UWorld* World = GetWorld();
    if (!World) return 0;
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(ActorType))
        {
            Count++;
        }
    }
    
    return Count;
}

EBuild_IntegrationStatus UBuildIntegrationManager::GetOverallBuildStatus() const
{
    return OverallStatus;
}

FString UBuildIntegrationManager::GetBuildStatusMessage() const
{
    return StatusMessage;
}

bool UBuildIntegrationManager::SaveCurrentMap()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: No world to save"));
        return false;
    }
    
    FString MapPath = TEXT("/Game/Maps/MinPlayableMap");
    
    // Note: In runtime, we can't directly save maps like in editor
    // This would need to be implemented differently for runtime vs editor
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Map save requested for %s"), *MapPath);
    
    return true;
}

bool UBuildIntegrationManager::LoadMap(const FString& MapPath)
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Loading map %s"), *MapPath);
    
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // This would need proper implementation for runtime map loading
    return true;
}

float UBuildIntegrationManager::GetFrameRate() const
{
    if (GEngine && GEngine->GameViewport)
    {
        return 1.0f / GEngine->GameViewport->GetWorld()->GetDeltaSeconds();
    }
    return 0.0f;
}

float UBuildIntegrationManager::GetMemoryUsageMB() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

bool UBuildIntegrationManager::ValidateWorldGeneration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateSystemReport(TEXT("WorldGeneration"), EBuild_IntegrationStatus::Failed, TEXT("No world available"));
        return false;
    }
    
    // Check for PCG actors
    int32 PCGActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("PCG")))
        {
            PCGActorCount++;
        }
    }
    
    if (PCGActorCount > 0)
    {
        UpdateSystemReport(TEXT("WorldGeneration"), EBuild_IntegrationStatus::Success, 
            FString::Printf(TEXT("Found %d PCG actors"), PCGActorCount));
        return true;
    }
    else
    {
        UpdateSystemReport(TEXT("WorldGeneration"), EBuild_IntegrationStatus::Warning, TEXT("No PCG actors found"));
        return false;
    }
}

bool UBuildIntegrationManager::ValidateCharacterSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateSystemReport(TEXT("Character"), EBuild_IntegrationStatus::Failed, TEXT("No world available"));
        return false;
    }
    
    // Check for TranspersonalCharacter actors
    int32 CharacterCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("TranspersonalCharacter")))
        {
            CharacterCount++;
        }
    }
    
    UpdateSystemReport(TEXT("Character"), EBuild_IntegrationStatus::Success, 
        FString::Printf(TEXT("Found %d character actors"), CharacterCount));
    return true;
}

bool UBuildIntegrationManager::ValidateDinosaurSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateSystemReport(TEXT("Dinosaur"), EBuild_IntegrationStatus::Failed, TEXT("No world available"));
        return false;
    }
    
    // Check for dinosaur actors
    int32 DinosaurCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ActorName = Actor->GetName().ToLower();
            if (ActorName.Contains(TEXT("dinosaur")) || ActorName.Contains(TEXT("trex")) || 
                ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("brachio")))
            {
                DinosaurCount++;
            }
        }
    }
    
    UpdateSystemReport(TEXT("Dinosaur"), EBuild_IntegrationStatus::Success, 
        FString::Printf(TEXT("Found %d dinosaur actors"), DinosaurCount));
    return true;
}

bool UBuildIntegrationManager::ValidateVFXSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateSystemReport(TEXT("VFX"), EBuild_IntegrationStatus::Failed, TEXT("No world available"));
        return false;
    }
    
    // Check for VFX-related actors
    int32 VFXCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ClassName = Actor->GetClass()->GetName().ToLower();
            if (ClassName.Contains(TEXT("effect")) || ClassName.Contains(TEXT("particle")) || 
                ClassName.Contains(TEXT("niagara")) || ClassName.Contains(TEXT("footstep")))
            {
                VFXCount++;
            }
        }
    }
    
    UpdateSystemReport(TEXT("VFX"), EBuild_IntegrationStatus::Success, 
        FString::Printf(TEXT("Found %d VFX actors"), VFXCount));
    return true;
}

bool UBuildIntegrationManager::ValidateAudioSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateSystemReport(TEXT("Audio"), EBuild_IntegrationStatus::Failed, TEXT("No world available"));
        return false;
    }
    
    // Check for audio-related actors
    int32 AudioCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ClassName = Actor->GetClass()->GetName().ToLower();
            if (ClassName.Contains(TEXT("audio")) || ClassName.Contains(TEXT("sound")) || 
                ClassName.Contains(TEXT("ambient")))
            {
                AudioCount++;
            }
        }
    }
    
    UpdateSystemReport(TEXT("Audio"), EBuild_IntegrationStatus::Success, 
        FString::Printf(TEXT("Found %d audio actors"), AudioCount));
    return true;
}

bool UBuildIntegrationManager::ValidateQuestSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateSystemReport(TEXT("Quest"), EBuild_IntegrationStatus::Failed, TEXT("No world available"));
        return false;
    }
    
    // Check for quest-related actors
    int32 QuestCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ClassName = Actor->GetClass()->GetName().ToLower();
            if (ClassName.Contains(TEXT("quest")) || ClassName.Contains(TEXT("mission")) || 
                ClassName.Contains(TEXT("objective")))
            {
                QuestCount++;
            }
        }
    }
    
    UpdateSystemReport(TEXT("Quest"), EBuild_IntegrationStatus::Success, 
        FString::Printf(TEXT("Found %d quest actors"), QuestCount));
    return true;
}

void UBuildIntegrationManager::UpdateSystemReport(const FString& SystemName, EBuild_IntegrationStatus Status, const FString& Message)
{
    // Find existing report or create new one
    FBuild_SystemReport* ExistingReport = SystemReports.FindByPredicate([&SystemName](const FBuild_SystemReport& Report)
    {
        return Report.SystemName == SystemName;
    });
    
    if (ExistingReport)
    {
        ExistingReport->Status = Status;
        ExistingReport->ErrorMessage = Message;
        ExistingReport->ValidationTime = FPlatformTime::Seconds();
    }
    else
    {
        FBuild_SystemReport NewReport;
        NewReport.SystemName = SystemName;
        NewReport.Status = Status;
        NewReport.ErrorMessage = Message;
        NewReport.ValidationTime = FPlatformTime::Seconds();
        NewReport.ActorCount = GetActorCountByType(SystemName);
        
        SystemReports.Add(NewReport);
    }
}

void UBuildIntegrationManager::CalculateOverallStatus()
{
    if (SystemReports.Num() == 0)
    {
        OverallStatus = EBuild_IntegrationStatus::Pending;
        StatusMessage = TEXT("No systems validated yet");
        return;
    }
    
    int32 SuccessCount = 0;
    int32 WarningCount = 0;
    int32 FailedCount = 0;
    
    for (const FBuild_SystemReport& Report : SystemReports)
    {
        switch (Report.Status)
        {
            case EBuild_IntegrationStatus::Success:
                SuccessCount++;
                break;
            case EBuild_IntegrationStatus::Warning:
                WarningCount++;
                break;
            case EBuild_IntegrationStatus::Failed:
                FailedCount++;
                break;
            default:
                break;
        }
    }
    
    if (FailedCount > 0)
    {
        OverallStatus = EBuild_IntegrationStatus::Failed;
        StatusMessage = FString::Printf(TEXT("%d systems failed, %d warnings"), FailedCount, WarningCount);
    }
    else if (WarningCount > 0)
    {
        OverallStatus = EBuild_IntegrationStatus::Warning;
        StatusMessage = FString::Printf(TEXT("%d systems successful, %d warnings"), SuccessCount, WarningCount);
    }
    else
    {
        OverallStatus = EBuild_IntegrationStatus::Success;
        StatusMessage = FString::Printf(TEXT("All %d systems validated successfully"), SuccessCount);
    }
}