#include "BuildSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Eng_BuildSystemManager.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BuildSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UEng_BuildSystemManager();

    // Core build system functionality
    UFUNCTION(BlueprintCallable, Category = "Build System")
    bool InitializeBuildSystem();

    UFUNCTION(BlueprintCallable, Category = "Build System")
    bool ValidateStructuralIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build System")
    void UpdateBuildQueue();

    UFUNCTION(BlueprintCallable, Category = "Build System")
    bool ProcessBuildRequest(const FString& BuildTarget);

    UFUNCTION(BlueprintCallable, Category = "Build System")
    void CleanupBuildArtifacts();

    UFUNCTION(BlueprintCallable, Category = "Build System")
    FString GetBuildSystemStatus() const;

protected:
    // Build system state
    UPROPERTY(BlueprintReadOnly, Category = "Build System")
    bool bBuildSystemInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Build System")
    int32 ActiveBuildTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Build System")
    float BuildProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Build System")
    FString CurrentBuildTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Build System")
    TArray<FString> BuildQueue;

    UPROPERTY(BlueprintReadOnly, Category = "Build System")
    TArray<FString> CompletedBuilds;

private:
    // Internal build system methods
    bool ValidateBuildEnvironment();
    void ProcessNextBuildTask();
    void NotifyBuildCompletion(const FString& Target);
    void HandleBuildError(const FString& ErrorMessage);
};

// Implementation
UEng_BuildSystemManager::UEng_BuildSystemManager()
{
    bBuildSystemInitialized = false;
    ActiveBuildTasks = 0;
    BuildProgress = 0.0f;
    CurrentBuildTarget = TEXT("");
}

bool UEng_BuildSystemManager::InitializeBuildSystem()
{
    if (bBuildSystemInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build System already initialized"));
        return true;
    }

    // Initialize build environment
    if (!ValidateBuildEnvironment())
    {
        UE_LOG(LogTemp, Error, TEXT("Build environment validation failed"));
        return false;
    }

    // Clear any existing build state
    BuildQueue.Empty();
    CompletedBuilds.Empty();
    ActiveBuildTasks = 0;
    BuildProgress = 0.0f;
    CurrentBuildTarget = TEXT("");

    bBuildSystemInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("Build System initialized successfully"));
    return true;
}

bool UEng_BuildSystemManager::ValidateStructuralIntegrity()
{
    if (!bBuildSystemInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build System not initialized"));
        return false;
    }

    // Validate core build components
    bool bIntegrityValid = true;

    // Check build queue consistency
    if (BuildQueue.Num() < 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Build queue in invalid state"));
        bIntegrityValid = false;
    }

    // Check active task count
    if (ActiveBuildTasks < 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Active build tasks count invalid"));
        ActiveBuildTasks = 0;
        bIntegrityValid = false;
    }

    // Validate build progress
    if (BuildProgress < 0.0f || BuildProgress > 1.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build progress out of range, clamping"));
        BuildProgress = FMath::Clamp(BuildProgress, 0.0f, 1.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("Build System structural integrity: %s"), 
           bIntegrityValid ? TEXT("VALID") : TEXT("ISSUES DETECTED"));
    
    return bIntegrityValid;
}

void UEng_BuildSystemManager::UpdateBuildQueue()
{
    if (!bBuildSystemInitialized)
    {
        return;
    }

    // Process pending build tasks
    if (BuildQueue.Num() > 0 && ActiveBuildTasks == 0)
    {
        ProcessNextBuildTask();
    }

    // Update build progress for active tasks
    if (ActiveBuildTasks > 0)
    {
        BuildProgress += 0.1f; // Simulate build progress
        BuildProgress = FMath::Clamp(BuildProgress, 0.0f, 1.0f);

        // Check if build is complete
        if (BuildProgress >= 1.0f)
        {
            NotifyBuildCompletion(CurrentBuildTarget);
        }
    }
}

bool UEng_BuildSystemManager::ProcessBuildRequest(const FString& BuildTarget)
{
    if (!bBuildSystemInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot process build request: System not initialized"));
        return false;
    }

    if (BuildTarget.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot process empty build target"));
        return false;
    }

    // Add to build queue
    BuildQueue.Add(BuildTarget);
    UE_LOG(LogTemp, Log, TEXT("Added build target to queue: %s"), *BuildTarget);

    // Start processing if no active tasks
    if (ActiveBuildTasks == 0)
    {
        UpdateBuildQueue();
    }

    return true;
}

void UEng_BuildSystemManager::CleanupBuildArtifacts()
{
    if (!bBuildSystemInitialized)
    {
        return;
    }

    // Clear completed builds older than threshold
    if (CompletedBuilds.Num() > 10)
    {
        int32 ItemsToRemove = CompletedBuilds.Num() - 10;
        CompletedBuilds.RemoveAt(0, ItemsToRemove);
        UE_LOG(LogTemp, Log, TEXT("Cleaned up %d old build artifacts"), ItemsToRemove);
    }

    // Reset build progress if no active tasks
    if (ActiveBuildTasks == 0)
    {
        BuildProgress = 0.0f;
        CurrentBuildTarget = TEXT("");
    }
}

FString UEng_BuildSystemManager::GetBuildSystemStatus() const
{
    if (!bBuildSystemInitialized)
    {
        return TEXT("Build System: NOT INITIALIZED");
    }

    FString Status = FString::Printf(
        TEXT("Build System: ACTIVE | Queue: %d | Active Tasks: %d | Progress: %.1f%% | Current: %s"),
        BuildQueue.Num(),
        ActiveBuildTasks,
        BuildProgress * 100.0f,
        *CurrentBuildTarget
    );

    return Status;
}

bool UEng_BuildSystemManager::ValidateBuildEnvironment()
{
    // Check if we have a valid world context
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    }

    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world context for build system"));
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("Build environment validation passed"));
    return true;
}

void UEng_BuildSystemManager::ProcessNextBuildTask()
{
    if (BuildQueue.Num() == 0)
    {
        return;
    }

    // Get next build target
    CurrentBuildTarget = BuildQueue[0];
    BuildQueue.RemoveAt(0);

    // Start build task
    ActiveBuildTasks = 1;
    BuildProgress = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("Started build task: %s"), *CurrentBuildTarget);
}

void UEng_BuildSystemManager::NotifyBuildCompletion(const FString& Target)
{
    // Mark build as complete
    CompletedBuilds.Add(Target);
    ActiveBuildTasks = 0;
    BuildProgress = 0.0f;
    CurrentBuildTarget = TEXT("");

    UE_LOG(LogTemp, Log, TEXT("Build completed: %s"), *Target);

    // Process next task if available
    if (BuildQueue.Num() > 0)
    {
        ProcessNextBuildTask();
    }
}

void UEng_BuildSystemManager::HandleBuildError(const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("Build error: %s"), *ErrorMessage);
    
    // Reset build state
    ActiveBuildTasks = 0;
    BuildProgress = 0.0f;
    CurrentBuildTarget = TEXT("");

    // Continue with next task if available
    if (BuildQueue.Num() > 0)
    {
        ProcessNextBuildTask();
    }
}