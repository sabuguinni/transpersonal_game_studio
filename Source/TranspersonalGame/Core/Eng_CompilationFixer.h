#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Eng_CompilationFixer.generated.h"

UENUM(BlueprintType)
enum class EEng_CompilationStatus : uint8
{
    Unknown UMETA(DisplayName = "Unknown"),
    Compiling UMETA(DisplayName = "Compiling"),
    Success UMETA(DisplayName = "Success"),
    Failed UMETA(DisplayName = "Failed"),
    Orphaned UMETA(DisplayName = "Orphaned Headers")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_OrphanedHeader
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString HeaderName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString FilePath;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    bool bHasCppFile;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 LineCount;

    FEng_OrphanedHeader()
    {
        HeaderName = "";
        FilePath = "";
        bHasCppFile = false;
        LineCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 TotalHeaders;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 TotalCppFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 OrphanedCount;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FEng_OrphanedHeader> OrphanedHeaders;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    EEng_CompilationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString LastError;

    FEng_CompilationReport()
    {
        TotalHeaders = 0;
        TotalCppFiles = 0;
        OrphanedCount = 0;
        Status = EEng_CompilationStatus::Unknown;
        LastError = "";
    }
};

/**
 * Engine Architect Compilation Fixer - Identifies and resolves orphaned headers
 * Automatically creates missing .cpp files and fixes compilation issues
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CompilationFixer : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CompilationFixer();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    FEng_CompilationReport AnalyzeCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool FixOrphanedHeaders(const TArray<FString>& HeadersToFix);

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool CreateMissingCppFile(const FString& HeaderPath);

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    TArray<FString> GetCriticalOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architect")
    void RunCompilationFix();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architect")
    FEng_CompilationReport LastReport;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architect")
    TArray<FString> CriticalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architect")
    bool bFixInProgress;

private:
    void InitializeCriticalSystems();
    FString GenerateStubCppContent(const FString& HeaderName);
    bool IsHeaderCritical(const FString& HeaderName);
    void LogCompilationStatus();
};

/**
 * Engine Architect Compilation Manager Actor
 * Provides in-editor tools for compilation management
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_CompilationManager : public AActor
{
    GENERATED_BODY()

public:
    AEng_CompilationManager();

protected:
    virtual void BeginPlay() override;

public:
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architect")
    void AnalyzeAndFixCompilation();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architect")
    void CreateMissingCppFiles();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architect")
    void ValidateArchitecture();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine Architect")
    class UStaticMeshComponent* VisualizationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Architect")
    bool bAutoFixOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Architect")
    float FixInterval;

private:
    FTimerHandle FixTimerHandle;
    void PerformPeriodicFix();
};