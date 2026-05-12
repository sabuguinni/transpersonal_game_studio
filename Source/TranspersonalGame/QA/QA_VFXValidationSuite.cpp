#include "QA_VFXValidationSuite.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UQA_VFXValidationSuite::UQA_VFXValidationSuite()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for performance monitoring
    
    // Initialize validation state
    bVFXSystemsValid = false;
    bNiagaraSystemsValid = false;
    bVFXMaterialsValid = false;
    bVFXPerformanceAcceptable = false;
    TotalNiagaraComponents = 0;
    TotalVFXMaterials = 0;
    CurrentVFXMemoryMB = 0.0f;
    
    LastValidationTime = 0.0f;
    bValidationInProgress = false;
    AccumulatedFrameTime = 0.0f;
    FrameCount = 0;
    AverageFrameTime = 0.0f;
}

void UQA_VFXValidationSuite::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXValidationSuite: Starting VFX validation system"));
    
    // Run initial validation
    ValidateVFXSystems();
}

void UQA_VFXValidationSuite::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Run periodic validation every 30 seconds
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastValidationTime > 30.0f && !bValidationInProgress)
    {
        ValidateVFXSystems();
        LastValidationTime = CurrentTime;
    }
}

bool UQA_VFXValidationSuite::ValidateVFXSystems()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA_VFXValidationSuite: Validation already in progress"));
        return false;
    }
    
    bValidationInProgress = true;
    ClearValidationResults();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXValidationSuite: Starting comprehensive VFX validation"));
    
    bool bAllTestsPassed = true;
    
    // Test 1: Validate Niagara systems
    if (!ValidateNiagaraSystems())
    {
        bAllTestsPassed = false;
        ValidationErrors.Add(TEXT("Niagara systems validation failed"));
    }
    
    // Test 2: Validate VFX materials
    if (!ValidateVFXMaterials())
    {
        bAllTestsPassed = false;
        ValidationErrors.Add(TEXT("VFX materials validation failed"));
    }
    
    // Test 3: Validate performance
    if (!ValidateVFXPerformance())
    {
        bAllTestsPassed = false;
        ValidationErrors.Add(TEXT("VFX performance validation failed"));
    }
    
    // Test 4: Validate Cretaceous VFX Controller
    if (!ValidateCretaceousVFXController())
    {
        bAllTestsPassed = false;
        ValidationErrors.Add(TEXT("Cretaceous VFX Controller validation failed"));
    }
    
    bVFXSystemsValid = bAllTestsPassed;
    
    if (bEnableDetailedLogging)
    {
        LogValidationResults();
    }
    
    bValidationInProgress = false;
    
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXValidationSuite: Validation complete. Result: %s"), 
           bAllTestsPassed ? TEXT("PASS") : TEXT("FAIL"));
    
    return bAllTestsPassed;
}

bool UQA_VFXValidationSuite::ValidateNiagaraSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXValidationSuite: Validating Niagara systems"));
    
    // Get asset registry
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    
    // Find all Niagara systems
    TArray<FAssetData> NiagaraAssets;
    AssetRegistry.GetAssetsByClass(UNiagaraSystem::StaticClass()->GetFName(), NiagaraAssets);
    
    TotalVFXMaterials = NiagaraAssets.Num(); // Store count for reference
    
    if (NiagaraAssets.Num() == 0)
    {
        ValidationWarnings.Add(TEXT("No Niagara systems found in project"));
        bNiagaraSystemsValid = false;
        return false;
    }
    
    // Validate Niagara components in the world
    if (!ValidateVFXActorComponents())
    {
        ValidationErrors.Add(TEXT("VFX actor components validation failed"));
        bNiagaraSystemsValid = false;
        return false;
    }
    
    bNiagaraSystemsValid = true;
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXValidationSuite: Found %d Niagara systems"), NiagaraAssets.Num());
    
    return true;
}

bool UQA_VFXValidationSuite::ValidateVFXMaterials()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXValidationSuite: Validating VFX materials"));
    
    // Get asset registry
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    
    // Find all materials
    TArray<FAssetData> MaterialAssets;
    AssetRegistry.GetAssetsByClass(UMaterial::StaticClass()->GetFName(), MaterialAssets);
    
    // Count VFX-related materials
    int32 VFXMaterialCount = 0;
    for (const FAssetData& Asset : MaterialAssets)
    {
        FString AssetName = Asset.AssetName.ToString();
        if (AssetName.Contains(TEXT("VFX")) || AssetName.Contains(TEXT("Particle")) || 
            AssetName.Contains(TEXT("Cretaceous")) || AssetName.Contains(TEXT("Effect")))
        {
            VFXMaterialCount++;
        }
    }
    
    TotalVFXMaterials = VFXMaterialCount;
    
    if (VFXMaterialCount == 0)
    {
        ValidationWarnings.Add(TEXT("No VFX-specific materials found"));
        bVFXMaterialsValid = false;
        return false;
    }
    
    bVFXMaterialsValid = true;
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXValidationSuite: Found %d VFX materials"), VFXMaterialCount);
    
    return true;
}

bool UQA_VFXValidationSuite::ValidateVFXPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXValidationSuite: Validating VFX performance"));
    
    // Get current memory usage
    CurrentVFXMemoryMB = GetVFXMemoryUsage();
    
    // Get active particle count
    int32 ActiveParticles = GetActiveParticleCount();
    
    // Get VFX frame time
    float VFXFrameTime = GetVFXFrameTime();
    
    bool bPerformanceAcceptable = true;
    
    // Check memory usage
    if (CurrentVFXMemoryMB > MaxAcceptableVFXMemoryMB)
    {
        ValidationWarnings.Add(FString::Printf(TEXT("VFX memory usage high: %.2f MB (max: %.2f MB)"), 
                                               CurrentVFXMemoryMB, MaxAcceptableVFXMemoryMB));
        bPerformanceAcceptable = false;
    }
    
    // Check particle count
    if (ActiveParticles > MaxAcceptableParticleCount)
    {
        ValidationWarnings.Add(FString::Printf(TEXT("Active particle count high: %d (max: %d)"), 
                                               ActiveParticles, MaxAcceptableParticleCount));
        bPerformanceAcceptable = false;
    }
    
    // Check frame time
    if (VFXFrameTime > MaxAcceptableVFXFrameTimeMS)
    {
        ValidationWarnings.Add(FString::Printf(TEXT("VFX frame time high: %.2f ms (max: %.2f ms)"), 
                                               VFXFrameTime, MaxAcceptableVFXFrameTimeMS));
        bPerformanceAcceptable = false;
    }
    
    bVFXPerformanceAcceptable = bPerformanceAcceptable;
    
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXValidationSuite: Performance check - Memory: %.2f MB, Particles: %d, Frame time: %.2f ms"), 
           CurrentVFXMemoryMB, ActiveParticles, VFXFrameTime);
    
    return bPerformanceAcceptable;
}

bool UQA_VFXValidationSuite::ValidateCretaceousVFXController()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXValidationSuite: Validating Cretaceous VFX Controller"));
    
    // This would normally check if the VFX_CretaceousParticleController class exists and functions
    // For now, we'll do a basic validation
    
    UWorld* World = GetWorld();
    if (!World)
    {
        ValidationErrors.Add(TEXT("No valid world context for VFX controller validation"));
        return false;
    }
    
    // Count actors with VFX-related names
    int32 VFXActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("VFX")) || ActorName.Contains(TEXT("Cretaceous")) || 
                ActorName.Contains(TEXT("Particle")) || ActorName.Contains(TEXT("Effect")))
            {
                VFXActorCount++;
            }
        }
    }
    
    if (VFXActorCount == 0)
    {
        ValidationWarnings.Add(TEXT("No VFX actors found in the world"));
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXValidationSuite: Found %d VFX actors"), VFXActorCount);
    return true;
}

float UQA_VFXValidationSuite::GetVFXMemoryUsage()
{
    // Simplified memory calculation - in a real implementation this would query actual VFX memory usage
    return 128.0f; // Placeholder value
}

int32 UQA_VFXValidationSuite::GetActiveParticleCount()
{
    int32 ParticleCount = 0;
    
    UWorld* World = GetWorld();
    if (World)
    {
        // Count Niagara components
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                TArray<UNiagaraComponent*> NiagaraComponents;
                Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
                ParticleCount += NiagaraComponents.Num() * 100; // Estimate 100 particles per component
            }
        }
    }
    
    return ParticleCount;
}

float UQA_VFXValidationSuite::GetVFXFrameTime()
{
    return AverageFrameTime;
}

bool UQA_VFXValidationSuite::ValidateVFXActorComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    TotalNiagaraComponents = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<UNiagaraComponent*> NiagaraComponents;
            Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
            TotalNiagaraComponents += NiagaraComponents.Num();
        }
    }
    
    return TotalNiagaraComponents > 0;
}

void UQA_VFXValidationSuite::UpdatePerformanceMetrics()
{
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    AccumulatedFrameTime += DeltaTime * 1000.0f; // Convert to milliseconds
    FrameCount++;
    
    if (FrameCount >= 60) // Update average every 60 frames
    {
        AverageFrameTime = AccumulatedFrameTime / FrameCount;
        AccumulatedFrameTime = 0.0f;
        FrameCount = 0;
    }
}

void UQA_VFXValidationSuite::LogValidationResults()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA VFX VALIDATION RESULTS ==="));
    UE_LOG(LogTemp, Warning, TEXT("VFX Systems Valid: %s"), bVFXSystemsValid ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Niagara Systems Valid: %s"), bNiagaraSystemsValid ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("VFX Materials Valid: %s"), bVFXMaterialsValid ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("VFX Performance Acceptable: %s"), bVFXPerformanceAcceptable ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Total Niagara Components: %d"), TotalNiagaraComponents);
    UE_LOG(LogTemp, Warning, TEXT("Total VFX Materials: %d"), TotalVFXMaterials);
    UE_LOG(LogTemp, Warning, TEXT("Current VFX Memory: %.2f MB"), CurrentVFXMemoryMB);
    
    for (const FString& Error : ValidationErrors)
    {
        UE_LOG(LogTemp, Error, TEXT("VALIDATION ERROR: %s"), *Error);
    }
    
    for (const FString& Warning : ValidationWarnings)
    {
        UE_LOG(LogTemp, Warning, TEXT("VALIDATION WARNING: %s"), *Warning);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END VALIDATION RESULTS ==="));
}

void UQA_VFXValidationSuite::ClearValidationResults()
{
    ValidationErrors.Empty();
    ValidationWarnings.Empty();
    bVFXSystemsValid = false;
    bNiagaraSystemsValid = false;
    bVFXMaterialsValid = false;
    bVFXPerformanceAcceptable = false;
}