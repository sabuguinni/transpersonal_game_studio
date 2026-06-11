#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Build_CompilationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_CompilationResult : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Success         UMETA(DisplayName = "Success"),
    Warning         UMETA(DisplayName = "Warning"),
    Error           UMETA(DisplayName = "Error"),
    Fatal           UMETA(DisplayName = "Fatal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CompilationInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    EBuild_CompilationResult Result = EBuild_CompilationResult::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 WarningCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FString> WarningMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float CompilationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString Timestamp;

    FBuild_CompilationInfo()
    {
        Result = EBuild_CompilationResult::Unknown;
        ModuleName = TEXT("");
        ErrorCount = 0;
        WarningCount = 0;
        CompilationTime = 0.0f;
        Timestamp = TEXT("");
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
    FBuild_CompilationInfo GetLastCompilationResult() const { return LastCompilationResult; }

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FBuild_CompilationInfo> GetCompilationHistory() const { return CompilationHistory; }

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool IsModuleCompiled(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void ValidateModuleCompilation(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void CheckAllModulesCompilation();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void GenerateCompilationReport();

    UFUNCTION(BlueprintCallable, Category = "Compilation", CallInEditor = true)
    void RefreshCompilationStatus();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FBuild_CompilationInfo LastCompilationResult;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FBuild_CompilationInfo> CompilationHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    TArray<FString> TrackedModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    int32 MaxHistoryEntries = 50;

private:
    void InitializeTrackedModules();
    void AddCompilationResult(const FBuild_CompilationInfo& Result);
    FString GetModuleBinaryPath(const FString& ModuleName);
    bool CheckBinaryExists(const FString& BinaryPath);
    void ParseCompilationLog(const FString& LogPath, FBuild_CompilationInfo& OutInfo);
    void LogCompilationStatus(const FBuild_CompilationInfo& Info);
};