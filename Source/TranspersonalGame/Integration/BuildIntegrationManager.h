#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/TimerHandle.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBuildIntegration, Log, All);

/**
 * Resultado de compilação
 */
UENUM(BlueprintType)
enum class EBuild_CompilationResult : uint8
{
    Unknown     UMETA(DisplayName = "Desconhecido"),
    Success     UMETA(DisplayName = "Sucesso"),
    Failed      UMETA(DisplayName = "Falhado"),
    InProgress  UMETA(DisplayName = "Em Progresso")
};

/**
 * Relatório de validação do build
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    FDateTime ValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    int32 TotalHeaderFiles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    int32 TotalCppFiles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    int32 OrphanHeaders = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    TArray<FString> OrphanHeadersList;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    EBuild_CompilationResult CompilationResult = EBuild_CompilationResult::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    int32 CompilationErrors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    FDateTime LastCompilationTime;

    FBuild_ValidationReport()
    {
        ValidationTime = FDateTime::MinValue();
        LastCompilationTime = FDateTime::MinValue();
    }
};

/**
 * Gestor de integração e build do projecto
 * Responsável por validar a estrutura do código, detectar problemas de compilação
 * e manter a integridade do build entre ciclos de desenvolvimento
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Valida a estrutura do projecto
     * Verifica se todos os .h têm .cpp correspondente
     * @return true se a estrutura está válida
     */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateProjectStructure();

    /**
     * Testa a compilação do projecto
     * @return true se a compilação foi bem-sucedida
     */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TestCompilation();

    /**
     * Obtém o relatório de validação actual
     * @return Relatório com estado detalhado
     */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_ValidationReport GetValidationReport() const;

    /**
     * Activa/desactiva a validação automática
     * @param bEnabled Se true, activa validação periódica
     */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SetValidationEnabled(bool bEnabled);

    /**
     * Força uma validação manual imediata
     */
    UFUNCTION(BlueprintCallable, Category = "Build Integration", CallInEditor)
    void ForceValidation();

    /**
     * Obtém lista de headers órfãos
     * @return Array com caminhos dos ficheiros .h sem .cpp
     */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetOrphanHeaders() const;

    /**
     * Obtém número de headers órfãos
     * @return Número de ficheiros .h sem .cpp correspondente
     */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    int32 GetOrphanHeaderCount() const;

    /**
     * Obtém resultado da última compilação
     * @return Estado da última tentativa de compilação
     */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuild_CompilationResult GetLastCompilationResult() const;

    /**
     * Obtém número de erros de compilação
     * @return Número de erros encontrados na última compilação
     */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    int32 GetCompilationErrorCount() const;

protected:
    /**
     * Validação periódica executada por timer
     */
    void PeriodicValidation();

private:
    /** Se a validação automática está activa */
    UPROPERTY(EditAnywhere, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    bool bIsEnabled;

    /** Intervalo entre validações automáticas (segundos) */
    UPROPERTY(EditAnywhere, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    float ValidationIntervalSeconds;

    /** Timer para validações periódicas */
    FTimerHandle ValidationTimerHandle;

    /** Timestamp da última validação */
    FDateTime LastValidationTime;

    /** Número total de ficheiros .h encontrados */
    int32 TotalHeaderFiles;

    /** Número total de ficheiros .cpp encontrados */
    int32 TotalCppFiles;

    /** Número de headers órfãos (.h sem .cpp) */
    int32 OrphanHeaders;

    /** Lista de caminhos dos headers órfãos */
    TArray<FString> OrphanHeadersList;

    /** Resultado da última compilação */
    EBuild_CompilationResult LastCompilationResult;

    /** Número de erros de compilação */
    int32 CompilationErrors;

    /** Timestamp da última compilação */
    FDateTime LastCompilationTime;
};