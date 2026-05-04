#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/TimerHandle.h"
#include "BuildIntegrationManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBuildIntegration, Log, All);

/**
 * Build Integration Manager - Agente #19
 * 
 * Sistema responsável por:
 * - Validar integridade de módulos C++
 * - Detectar headers órfãos (sem .cpp correspondente)
 * - Monitorizar dependências entre módulos
 * - Reportar erros de compilação
 * - Garantir que o build está sempre funcional
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

    /** Executar validação completa do build */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void PerformFullValidation();

    /** Forçar validação imediata */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ForceValidation();

    /** Verificar se a validação está a passar */
    UFUNCTION(BlueprintPure, Category = "Build Integration")
    bool IsValidationPassing() const;

    /** Obter resumo da validação */
    UFUNCTION(BlueprintPure, Category = "Build Integration")
    FString GetValidationSummary() const;

    /** Obter lista de erros de validação */
    UFUNCTION(BlueprintPure, Category = "Build Integration")
    TArray<FString> GetValidationErrors() const;

    /** Activar/desactivar validação automática */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SetValidationEnabled(bool bEnabled);

protected:
    /** Validar estrutura de módulos */
    void ValidateModuleStructure();

    /** Validar headers órfãos */
    void ValidateOrphanHeaders();

    /** Validar dependências entre módulos */
    void ValidateModuleDependencies();

    /** Adicionar erro de validação */
    void AddValidationError(const FString& ErrorMessage);

private:
    /** Timer para validação periódica */
    FTimerHandle ValidationTimerHandle;

    /** Lista de erros encontrados */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation", meta = (AllowPrivateAccess = "true"))
    TArray<FString> ValidationErrors;

public:
    /** Executar validação automática no startup */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    bool bAutoValidateOnStartup;

    /** Activar verificações de compilação */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    bool bEnableCompilationChecks;

    /** Activar validação de módulos */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    bool bEnableModuleValidation;

    /** Intervalo entre validações (segundos) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration", meta = (ClampMin = "10.0", ClampMax = "300.0"))
    float ValidationIntervalSeconds;

    /** Máximo de erros a reportar */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration", meta = (ClampMin = "10", ClampMax = "100"))
    int32 MaxValidationErrors;

    /** Estatísticas de validação */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics")
    int32 TotalModulesFound;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics")
    int32 ValidModulesCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics")
    int32 OrphanHeadersCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics")
    int32 CompilationErrorsCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics")
    float LastValidationTime;
};