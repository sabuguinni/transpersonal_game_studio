#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TimerHandle.h"
#include "TranspersonalGame/SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBuildIntegration, Log, All);

/**
 * Sistema de gestão de integração e build do projeto
 * Responsável por validar a integridade do código, detectar problemas
 * e garantir que todos os sistemas estão funcionais
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    /** Executar validação completa de integração */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void PerformIntegrationValidation();
    
    /** Forçar validação imediata */
    UFUNCTION(BlueprintCallable, Category = "Build Integration", CallInEditor)
    void ForceValidation();
    
    /** Obter relatório detalhado do estado de integração */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FString GetIntegrationReport() const;
    
    /** Configurar intervalo entre validações automáticas */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SetValidationInterval(float NewInterval);

    // Propriedades de estado (somente leitura via Blueprint)
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    EBuild_IntegrationState IntegrationState;
    
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 TotalModulesLoaded;
    
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 FailedModulesCount;
    
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 OrphanHeadersCount;
    
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 DuplicateActorsCount;
    
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float LastValidationTime;

private:
    /** Validar módulos C++ carregados */
    void ValidateLoadedModules();
    
    /** Verificar headers sem implementação .cpp */
    void CheckOrphanHeaders();
    
    /** Verificar actores duplicados no mapa */
    void CheckDuplicateActors();
    
    /** Validar dependências entre sistemas */
    void ValidateSystemDependencies();
    
    /** Validar sistema de geração de mundo */
    bool ValidateWorldGeneration();
    
    /** Validar sistema de personagem */
    bool ValidateCharacterSystem();
    
    /** Validar GameMode */
    bool ValidateGameMode();
    
    /** Atualizar estado de integração baseado nos resultados */
    void UpdateIntegrationState();

    // Configuração
    UPROPERTY(EditAnywhere, Category = "Build Integration", meta = (ClampMin = "10.0", ClampMax = "300.0"))
    float ValidationInterval;
    
    // Timer para validações automáticas
    FTimerHandle ValidationTimerHandle;
    
    // Resultados da validação de sistemas
    TMap<FString, bool> SystemValidationResults;
};