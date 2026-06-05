#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Build_CompilationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_CompilationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Compiling       UMETA(DisplayName = "Compiling"),
    Success         UMETA(DisplayName = "Success"),
    Failed          UMETA(DisplayName = "Failed"),
    Warning         UMETA(DisplayName = "Warning")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CompilationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    EBuild_CompilationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 WarningCount;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float CompilationTime;

    FBuild_CompilationResult()
    {
        Status = EBuild_CompilationStatus::Unknown;
        ModuleName = TEXT("");
        ErrorCount = 0;
        WarningCount = 0;
        CompilationTime = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_CompilationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool StartCompilation(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    FBuild_CompilationResult GetCompilationResult(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FBuild_CompilationResult> GetAllCompilationResults() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool IsCompilationInProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void ClearCompilationHistory();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateModuleDependencies(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    int32 GetTotalModuleCount() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FBuild_CompilationResult> CompilationHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    bool bCompilationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString CurrentCompilingModule;

private:
    void OnCompilationComplete(const FString& ModuleName, bool bSuccess);
    bool CheckModuleExists(const FString& ModuleName);
    void LogCompilationResult(const FBuild_CompilationResult& Result);
};