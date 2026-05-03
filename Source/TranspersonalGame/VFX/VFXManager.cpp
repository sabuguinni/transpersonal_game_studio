#include "VFXManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"

void UVFX_Manager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Inicializando sistema de efeitos visuais"));

    // Inicializar dados de configuração
    InitializeFootstepData();
    InitializeWeatherData();
    InitializeCombatData();

    // Limpar arrays
    ActiveEffects.Empty();
    CurrentWeatherEffect = nullptr;
    CurrentCampfireEffect = nullptr;

    UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Sistema inicializado com sucesso"));
}

void UVFX_Manager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Desinicializando sistema"));

    // Parar todos os efeitos activos
    StopWeatherEffect();
    StopCampfireEffect();

    // Limpar todos os efeitos activos
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    ActiveEffects.Empty();

    Super::Deinitialize();
}

void UVFX_Manager::InitializeFootstepData()
{
    // Configurar dados de pegadas por espécie de dinossauro
    FVFX_FootstepData TRexData;
    TRexData.ImpactIntensity = 3.0f;
    TRexData.EffectDuration = 4.0f;
    TRexData.ParticleRadius = 300.0f;
    DinosaurFootstepData.Add(EDinosaurSpecies::TRex, TRexData);

    FVFX_FootstepData RaptorData;
    RaptorData.ImpactIntensity = 1.0f;
    RaptorData.EffectDuration = 2.0f;
    RaptorData.ParticleRadius = 80.0f;
    DinosaurFootstepData.Add(EDinosaurSpecies::Velociraptor, RaptorData);

    FVFX_FootstepData BrachioData;
    BrachioData.ImpactIntensity = 5.0f;
    BrachioData.EffectDuration = 6.0f;
    BrachioData.ParticleRadius = 500.0f;
    DinosaurFootstepData.Add(EDinosaurSpecies::Brachiosaurus, BrachioData);

    FVFX_FootstepData TriceratopsData;
    TriceratopsData.ImpactIntensity = 2.5f;
    TriceratopsData.EffectDuration = 3.5f;
    TriceratopsData.ParticleRadius = 200.0f;
    DinosaurFootstepData.Add(EDinosaurSpecies::Triceratops, TriceratopsData);

    // Configurar dados de pegadas por bioma
    FVFX_FootstepData SwampData;
    SwampData.ImpactIntensity = 0.8f; // Menos poeira na lama
    SwampData.ParticleRadius = 120.0f;
    BiomeFootstepData.Add(EBiomeType::Swamp, SwampData);

    FVFX_FootstepData ForestData;
    ForestData.ImpactIntensity = 0.6f; // Solo macio da floresta
    ForestData.ParticleRadius = 100.0f;
    BiomeFootstepData.Add(EBiomeType::Forest, ForestData);

    FVFX_FootstepData SavannaData;
    SavannaData.ImpactIntensity = 1.2f; // Terra seca levanta mais poeira
    SavannaData.ParticleRadius = 150.0f;
    BiomeFootstepData.Add(EBiomeType::Savanna, SavannaData);

    FVFX_FootstepData DesertData;
    DesertData.ImpactIntensity = 1.5f; // Areia levanta muito pó
    DesertData.ParticleRadius = 200.0f;
    BiomeFootstepData.Add(EBiomeType::Desert, DesertData);

    FVFX_FootstepData MountainData;
    MountainData.ImpactIntensity = 0.4f; // Rocha produz menos partículas
    MountainData.ParticleRadius = 80.0f;
    BiomeFootstepData.Add(EBiomeType::SnowyMountain, MountainData);

    UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Dados de pegadas inicializados para %d espécies e %d biomas"), 
           DinosaurFootstepData.Num(), BiomeFootstepData.Num());
}

void UVFX_Manager::InitializeWeatherData()
{
    // Configurar sistemas de tempo
    // Nota: Os caminhos dos assets Niagara serão definidos quando os sistemas forem criados
    WeatherData.WeatherIntensity = 0.5f;

    UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Dados de tempo inicializados"));
}

void UVFX_Manager::InitializeCombatData()
{
    // Configurar sistemas de combate
    // Nota: Os caminhos dos assets Niagara serão definidos quando os sistemas forem criados

    UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Dados de combate inicializados"));
}

void UVFX_Manager::PlayFootstepEffect(const FVector& Location, EDinosaurSpecies Species, EBiomeType BiomeType)
{
    // Obter dados de configuração
    FVFX_FootstepData* SpeciesData = DinosaurFootstepData.Find(Species);
    FVFX_FootstepData* BiomeData = BiomeFootstepData.Find(BiomeType);

    if (!SpeciesData || !BiomeData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Dados de pegadas não encontrados para espécie %d ou bioma %d"), 
               (int32)Species, (int32)BiomeType);
        return;
    }

    // Calcular intensidade combinada
    float CombinedIntensity = SpeciesData->ImpactIntensity * BiomeData->ImpactIntensity;
    float CombinedRadius = FMath::Max(SpeciesData->ParticleRadius, BiomeData->ParticleRadius);
    float CombinedDuration = SpeciesData->EffectDuration;

    UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Reproduzir efeito de pegada em %s com intensidade %.2f e raio %.1f"), 
           *Location.ToString(), CombinedIntensity, CombinedRadius);

    // TODO: Quando os sistemas Niagara estiverem criados, spawnar o efeito aqui
    // UNiagaraComponent* Effect = SpawnNiagaraEffect(FootstepSystem, Location);
    // if (Effect)
    // {
    //     Effect->SetFloatParameter(TEXT("Intensity"), CombinedIntensity);
    //     Effect->SetFloatParameter(TEXT("Radius"), CombinedRadius);
    //     RegisterActiveEffect(Effect);
    // }
}

void UVFX_Manager::PlayPlayerFootstepEffect(const FVector& Location, EBiomeType BiomeType)
{
    // Usar dados de pegadas humanas (mais leves que dinossauros)
    FVFX_FootstepData* BiomeData = BiomeFootstepData.Find(BiomeType);

    if (!BiomeData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Dados de bioma não encontrados para %d"), (int32)BiomeType);
        return;
    }

    // Reduzir intensidade para pegadas humanas
    float PlayerIntensity = BiomeData->ImpactIntensity * 0.2f;
    float PlayerRadius = BiomeData->ParticleRadius * 0.3f;

    UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Reproduzir efeito de pegada do jogador em %s"), *Location.ToString());

    // TODO: Spawnar efeito de pegada do jogador
}

void UVFX_Manager::StartWeatherEffect(EWeatherType WeatherType, float Intensity)
{
    // Parar efeito anterior se existir
    StopWeatherEffect();

    UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Iniciar efeito de tempo %d com intensidade %.2f"), 
           (int32)WeatherType, Intensity);

    // TODO: Spawnar sistema de tempo baseado no tipo
    // switch (WeatherType)
    // {
    // case EWeatherType::Rain:
    //     CurrentWeatherEffect = SpawnNiagaraEffect(WeatherData.RainSystem.LoadSynchronous(), FVector::ZeroVector);
    //     break;
    // case EWeatherType::Snow:
    //     CurrentWeatherEffect = SpawnNiagaraEffect(WeatherData.SnowSystem.LoadSynchronous(), FVector::ZeroVector);
    //     break;
    // }

    WeatherData.WeatherIntensity = Intensity;
}

void UVFX_Manager::StopWeatherEffect()
{
    if (IsValid(CurrentWeatherEffect))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Parar efeito de tempo"));
        
        UnregisterActiveEffect(CurrentWeatherEffect);
        CurrentWeatherEffect->DestroyComponent();
        CurrentWeatherEffect = nullptr;
    }
}

void UVFX_Manager::UpdateWeatherIntensity(float NewIntensity)
{
    WeatherData.WeatherIntensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);

    if (IsValid(CurrentWeatherEffect))
    {
        CurrentWeatherEffect->SetFloatParameter(TEXT("Intensity"), WeatherData.WeatherIntensity);
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Actualizar intensidade do tempo para %.2f"), WeatherData.WeatherIntensity);
    }
}

void UVFX_Manager::PlayBloodSplatterEffect(const FVector& Location, const FVector& ImpactDirection)
{
    UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Reproduzir efeito de sangue em %s"), *Location.ToString());

    // TODO: Spawnar efeito de sangue
    // UNiagaraComponent* Effect = SpawnNiagaraEffect(CombatData.BloodSplatterSystem.LoadSynchronous(), Location);
    // if (Effect)
    // {
    //     Effect->SetVectorParameter(TEXT("ImpactDirection"), ImpactDirection);
    //     RegisterActiveEffect(Effect);
    // }
}

void UVFX_Manager::PlayWeaponImpactEffect(const FVector& Location, const FVector& ImpactDirection)
{
    UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Reproduzir efeito de impacto de arma em %s"), *Location.ToString());

    // TODO: Spawnar efeito de impacto de arma
}

void UVFX_Manager::PlayCampfireEffect(const FVector& Location)
{
    // Parar fogueira anterior se existir
    StopCampfireEffect();

    UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Iniciar efeito de fogueira em %s"), *Location.ToString());

    // TODO: Spawnar efeito de fogueira
    // CurrentCampfireEffect = SpawnNiagaraEffect(CampfireSystem, Location);
    // if (CurrentCampfireEffect)
    // {
    //     RegisterActiveEffect(CurrentCampfireEffect);
    // }
}

void UVFX_Manager::StopCampfireEffect()
{
    if (IsValid(CurrentCampfireEffect))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Parar efeito de fogueira"));
        
        UnregisterActiveEffect(CurrentCampfireEffect);
        CurrentCampfireEffect->DestroyComponent();
        CurrentCampfireEffect = nullptr;
    }
}

void UVFX_Manager::CleanupExpiredEffects()
{
    int32 RemovedCount = 0;
    
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        
        if (!IsValid(Effect) || !Effect->IsActive())
        {
            if (IsValid(Effect))
            {
                Effect->DestroyComponent();
            }
            ActiveEffects.RemoveAt(i);
            RemovedCount++;
        }
    }

    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Limpeza removeu %d efeitos expirados"), RemovedCount);
    }
}

int32 UVFX_Manager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

UNiagaraComponent* UVFX_Manager::SpawnNiagaraEffect(UNiagaraSystem* System, const FVector& Location, const FRotator& Rotation)
{
    if (!System)
    {
        UE_LOG(LogTemp, Error, TEXT("VFX Manager - Sistema Niagara nulo fornecido"));
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("VFX Manager - Mundo não encontrado"));
        return nullptr;
    }

    // Spawnar componente Niagara
    UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World, System, Location, Rotation);

    if (NiagaraComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Sistema Niagara spawnado com sucesso em %s"), *Location.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("VFX Manager - Falha ao spawnar sistema Niagara"));
    }

    return NiagaraComponent;
}

void UVFX_Manager::RegisterActiveEffect(UNiagaraComponent* Effect)
{
    if (IsValid(Effect))
    {
        ActiveEffects.Add(Effect);
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Efeito registado. Total activos: %d"), ActiveEffects.Num());
    }
}

void UVFX_Manager::UnregisterActiveEffect(UNiagaraComponent* Effect)
{
    if (ActiveEffects.Contains(Effect))
    {
        ActiveEffects.Remove(Effect);
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager - Efeito removido. Total activos: %d"), ActiveEffects.Num());
    }
}